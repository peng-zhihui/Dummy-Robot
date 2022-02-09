
import sys
import platform
import threading
import fibre

def did_discover_device(device,
                        interactive_variables, discovered_devices,
                        branding_short, branding_long,
                        logger, app_shutdown_token):
    """
    Handles the discovery of new devices by displaying a
    message and making the device available to the interactive
    console
    """
    serial_number = '{:012X}'.format(device.serial_number) if hasattr(device, 'serial_number') else "[unknown serial number]"
    if serial_number in discovered_devices:
        verb = "Reconnected"
        index = discovered_devices.index(serial_number)
    else:
        verb = "Connected"
        discovered_devices.append(serial_number)
        index = len(discovered_devices) - 1
    interactive_name = branding_short + str(index)

    # Publish new device to interactive console
    interactive_variables[interactive_name] = device
    globals()[interactive_name] = device # Add to globals so tab complete works
    logger.notify("{} to {} {} as {}".format(verb, branding_long, serial_number, interactive_name))

    # Subscribe to disappearance of the device
    device.__channel__._channel_broken.subscribe(lambda: did_lose_device(interactive_name, logger, app_shutdown_token))

def did_lose_device(interactive_name, logger, app_shutdown_token):
    """
    Handles the disappearance of a device by displaying
    a message.
    """
    if not app_shutdown_token.is_set():
        logger.warn("Oh no {} disappeared".format(interactive_name))

def launch_shell(args,
                interactive_variables,
                print_banner, print_help,
                logger, app_shutdown_token,
                branding_short="dev", branding_long="device"):
    """
    Launches an interactive python or IPython command line
    interface.
    As devices are connected they are made available as
    "dev0", "dev1", ...
    The names of the variables can be customized by setting branding_short.
    """

    discovered_devices = []
    globals().update(interactive_variables)

    # Connect to device
    logger.debug("Waiting for {}...".format(branding_long))
    fibre.find_all(args.path, args.serial_number,
                    lambda dev: did_discover_device(dev, interactive_variables, discovered_devices, branding_short, branding_long, logger, app_shutdown_token),
                    app_shutdown_token,
                    app_shutdown_token,
                    logger=logger)

    # Check if IPython is installed
    if args.no_ipython:
        use_ipython = False
    else:
        try:
            import IPython
            use_ipython = True
        except:
            print("Warning: you don't have IPython installed.")
            print("If you want to have an improved interactive console with pretty colors,")
            print("you should install IPython\n")
            use_ipython = False

    interactive_variables["help"] = lambda: print_help(args, len(discovered_devices) > 0)

    # If IPython is installed, embed IPython shell, otherwise embed regular shell
    if use_ipython:
        help = lambda: print_help(args, len(discovered_devices) > 0) # Override help function # pylint: disable=W0612
        locals()['__name__'] = globals()['__name__'] # to fix broken "%run -i script.py"
        console = IPython.terminal.embed.InteractiveShellEmbed(banner1='')
        console.runcode = console.run_code # hack to make IPython look like the regular console
        interact = console
    else:
        # Enable tab complete if possible
        try:
            import readline # Works only on Unix
            readline.parse_and_bind("tab: complete")
        except:
            sudo_prefix = "" if platform.system() == "Windows" else "sudo "
            print("Warning: could not enable tab-complete. User experience will suffer.\n"
                "Run `{}pip install readline` and then restart this script to fix this."
                .format(sudo_prefix))

        import code
        console = code.InteractiveConsole(locals=interactive_variables)
        interact = lambda: console.interact(banner='')

    # install hook to hide ChannelBrokenException
    console.runcode('import sys')
    console.runcode('superexcepthook = sys.excepthook')
    console.runcode('def newexcepthook(ex_class,ex,trace):\n'
                    '  if ex_class.__module__ + "." + ex_class.__name__ != "fibre.ChannelBrokenException":\n'
                    '    superexcepthook(ex_class,ex,trace)')
    console.runcode('sys.excepthook=newexcepthook')


    # Launch shell
    print_banner()
    logger._skip_bottom_line = True
    interact()
    app_shutdown_token.set()
