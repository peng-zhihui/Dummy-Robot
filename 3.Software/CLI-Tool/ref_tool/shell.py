import fibre
import ref_tool
from ref_tool.utils import start_liveplotter

def print_banner():
    print('Please connect your Dummy-Robot.')
    print('You can also type help() or quit().')

def print_help(args, have_devices):
    print('')
    if have_devices:
        print('Connect your REF-Unit to {} and power it up.'.format(args.path))
        print('After that, the following message should appear:')
        print('  "Connected to REF-Unit [serial number] as odrv0"')
        print('')
        print('Once the REF-Unit is connected, type "ref0." and press <tab>')
    else:
        print('Type "ref0." and press <tab>')
    print('This will present you with all the properties that you can reference')
    print('')
    print('For example: "odrv0.motor0.encoder.pos_estimate"')
    print('will print the current encoder position on motor 0')
    print('and "odrv0.motor0.pos_setpoint = 10000"')
    print('will send motor0 to 10000')
    print('')


interactive_variables = {}

discovered_devices = []

def did_discover_device(odrive, logger, app_shutdown_token):
    """
    Handles the discovery of new devices by displaying a
    message and making the device available to the interactive
    console
    """
    serial_number = odrive.serial_number if hasattr(odrive, 'serial_number') else "[unknown serial number]"
    if serial_number in discovered_devices:
        verb = "Reconnected"
        index = discovered_devices.index(serial_number)
    else:
        verb = "Connected"
        discovered_devices.append(serial_number)
        index = len(discovered_devices) - 1
    interactive_name = "odrv" + str(index)

    # Publish new ODrive to interactive console
    interactive_variables[interactive_name] = odrive
    globals()[interactive_name] = odrive # Add to globals so tab complete works
    logger.notify("{} to ODrive {:012X} as {}".format(verb, serial_number, interactive_name))

    # Subscribe to disappearance of the device
    odrive.__channel__._channel_broken.subscribe(lambda: did_lose_device(interactive_name, logger, app_shutdown_token))

def did_lose_device(interactive_name, logger, app_shutdown_token):
    """
    Handles the disappearance of a device by displaying
    a message.
    """
    if not app_shutdown_token.is_set():
        logger.warn("Oh no {} disappeared".format(interactive_name))

def launch_shell(args, logger, app_shutdown_token):
    """
    Launches an interactive python or IPython command line
    interface.
    As ODrives are connected they are made available as
    "odrv0", "odrv1", ...
    """

    interactive_variables = {
        'start_liveplotter': start_liveplotter,
    }

    fibre.launch_shell(args,
                       interactive_variables,
                       print_banner, print_help,
                       logger, app_shutdown_token,
                       branding_short="dummy", branding_long="Dummy-Robot")
