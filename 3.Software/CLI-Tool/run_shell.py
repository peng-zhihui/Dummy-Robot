#!/usr/bin/env python3
from __future__ import print_function

import argparse
import os
import sys
import time

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(
    os.path.realpath(__file__))),
    "Firmware", "fibre", "python"))
from fibre import Logger, Event
import ref_tool
from ref_tool.configuration import *

old_print = print


def print(*args, **kwargs):
    kwargs.pop('flush', False)
    old_print(*args, **kwargs)
    file = kwargs.get('file', sys.stdout)
    file.flush() if file is not None else sys.stdout.flush()


script_path = os.path.dirname(os.path.realpath(__file__))

## Parse arguments ##
parser = argparse.ArgumentParser(description='Robot-Embedded-Framework command line utility\n',
                                 formatter_class=argparse.RawTextHelpFormatter)
subparsers = parser.add_subparsers(help='sub-command help', dest='command')
shell_parser = subparsers.add_parser('shell',
                                     help='Drop into an interactive python shell that lets you interact with the ODrive(s)')
shell_parser.add_argument("--no-ipython", action="store_true",
                          help="Use the regular Python shell "
                               "instead of the IPython shell, "
                               "even if IPython is installed.")
subparsers.add_parser('liveplotter', help="For plotting of REF parameters (i.e. position) in real time")

# General arguments
parser.add_argument("-p", "--path", metavar="PATH", action="store",
                    help="The path(s) where REF-board(s) should be discovered.\n"
                         "By default the script will connect to any REF-board on USB.\n\n"
                         "To select a specific USB device:\n"
                         "  --path usb:BUS:DEVICE\n"
                         "usbwhere BUS and DEVICE are the bus and device numbers as shown in `lsusb`.\n\n"
                         "To select a specific serial port:\n"
                         "  --path serial:PATH\n"
                         "where PATH is the path of the serial port. For example \"/dev/ttyUSB0\".\n"
                         "You can use `ls /dev/tty*` to find the correct port.\n\n"
                         "You can combine USB and serial specs by separating them with a comma (no space!)\n"
                         "Example:\n"
                         "  --path usb,serial:/dev/ttyUSB0\n"
                         "means \"discover any USB device or a serial device on /dev/ttyUSB0\"")
parser.add_argument("-s", "--serial-number", action="store",
                    help="The 12-digit serial number of the device. "
                         "This is a string consisting of 12 upper case hexadecimal "
                         "digits as displayed in lsusb. \n"
                         "    example: 385F324D3037\n"
                         "You can list all devices connected to USB by running\n"
                         "(lsusb -d 1209:0d32 -v; lsusb -d 0483:df11 -v) | grep iSerial\n"
                         "If omitted, any device is accepted.")
parser.add_argument("-v", "--verbose", action="store_true",
                    help="print debug information")
parser.add_argument("--version", action="store_true",
                    help="print version information and exit")

parser.set_defaults(path="usb")
args = parser.parse_args()

# Default command
if args.command is None:
    args.command = 'shell'
    args.no_ipython = False
logger = Logger(verbose=args.verbose)

app_shutdown_token = Event()

try:
    if args.command == 'shell':
        # if ".dev" in ref_tool.__version__:
        #     print("")
        #     logger.warn("Developer Preview")
        #     print("")
        import ref_tool.shell

        ref_tool.shell.launch_shell(args, logger, app_shutdown_token)

    elif args.command == 'liveplotter':
        from ref_tool.utils import start_liveplotter

        print("Waiting for ODrive...")
        ref_unit = ref_tool.find_any(path=args.path, serial_number=args.serial_number,
                                     search_cancellation_token=app_shutdown_token,
                                     channel_termination_token=app_shutdown_token)

        # If you want to plot different values, change them here.
        # You can plot any number of values concurrently.
        cancellation_token = start_liveplotter(lambda: [
            ref_unit.axis0.encoder.pos_estimate,
            ref_unit.axis1.encoder.pos_estimate,
        ])

        print("Showing plot. Press Ctrl+C to exit.")
        while not cancellation_token.is_set():
            time.sleep(1)

    elif args.command == 'drv-status':
        from ref_tool.utils import print_drv_regs

        print("Waiting for ODrive...")
        ref_unit = ref_tool.find_any(path=args.path, serial_number=args.serial_number,
                                     search_cancellation_token=app_shutdown_token,
                                     channel_termination_token=app_shutdown_token)
        print_drv_regs("Motor 0", ref_unit.axis0.motor)
        print_drv_regs("Motor 1", ref_unit.axis1.motor)

    elif args.command == 'rate-test':
        from ref_tool.utils import rate_test

        print("Waiting for ODrive...")
        ref_unit = ref_tool.find_any(path=args.path, serial_number=args.serial_number,
                                     search_cancellation_token=app_shutdown_token,
                                     channel_termination_token=app_shutdown_token)
        rate_test(ref_unit)

    elif args.command == 'udev-setup':
        from ref_tool.version import setup_udev_rules

        setup_udev_rules(logger)

    elif args.command == 'generate-code':
        from ref_tool.code_generator import generate_code

        ref_unit = ref_tool.find_any(path=args.path, serial_number=args.serial_number,
                                     channel_termination_token=app_shutdown_token)
        generate_code(ref_unit, args.template, args.output)

    elif args.command == 'backup-config':
        from ref_tool.configuration import backup_config

        print("Waiting for ODrive...")
        ref_unit = ref_tool.find_any(path=args.path, serial_number=args.serial_number,
                                     search_cancellation_token=app_shutdown_token,
                                     channel_termination_token=app_shutdown_token)
        backup_config(ref_unit, args.file, logger)

    elif args.command == 'restore-config':
        from ref_tool.configuration import restore_config

        print("Waiting for ODrive...")
        ref_unit = ref_tool.find_any(path=args.path, serial_number=args.serial_number,
                                     search_cancellation_token=app_shutdown_token,
                                     channel_termination_token=app_shutdown_token)
        restore_config(ref_unit, args.file, logger)

    else:
        raise Exception("unknown command: " + args.command)

except OperationAbortedException:
    logger.info("Operation aborted.")
finally:
    app_shutdown_token.set()
