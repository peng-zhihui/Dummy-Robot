
import json
import os
import tempfile
import fibre.remote_object
from ref_tool.utils import OperationAbortedException, yes_no_prompt

def get_dict(obj, is_config_object):
    result = {}
    for (k,v) in obj._remote_attributes.items():
        if isinstance(v, fibre.remote_object.RemoteProperty) and is_config_object:
            result[k] = v.get_value()
        elif isinstance(v, fibre.remote_object.RemoteObject):
            sub_dict = get_dict(v, k == 'config')
            if sub_dict != {}:
                result[k] = sub_dict
    return result

def set_dict(obj, path, config_dict):
    errors = []
    for (k,v) in config_dict.items():
        name = path + ("." if path != "" else "") + k
        if not k in obj._remote_attributes:
            errors.append("Could not restore {}: property not found on device".format(name))
            continue
        remote_attribute = obj._remote_attributes[k]
        if isinstance(remote_attribute, fibre.remote_object.RemoteObject):
            errors += set_dict(remote_attribute, name, v)
        else:
            try:
                remote_attribute.set_value(v)
            except Exception as ex:
                errors.append("Could not restore {}: {}".format(name, str(ex)))
    return errors

def get_temp_config_filename(device):
    serial_number = fibre.utils.get_serial_number_str(device)
    safe_serial_number = ''.join(filter(str.isalnum, serial_number))
    return os.path.join(tempfile.gettempdir(), 'ref_tool-config-{}.json'.format(safe_serial_number))

def backup_config(device, filename, logger):
    """
    Exports the configuration of an ODrive to a JSON file.
    If no file name is provided, the file is placed into a
    temporary directory.
    """

    if filename is None:
        filename = get_temp_config_filename(device)

    logger.info("Saving configuration to {}...".format(filename))

    if os.path.exists(filename):
        if not yes_no_prompt("The file {} already exists. Do you want to override it?".format(filename), True):
            raise OperationAbortedException()

    data = get_dict(device, False)
    with open(filename, 'w') as file:
        json.dump(data, file)
    logger.info("Configuration saved.")

def restore_config(device, filename, logger):
    """
    Restores the configuration stored in a file 
    """

    if filename is None:
        filename = get_temp_config_filename(device)

    with open(filename) as file:
        data = json.load(file)

    logger.info("Restoring configuration from {}...".format(filename))
    errors = set_dict(device, "", data)

    for error in errors:
        logger.info(error)
    if errors:
        logger.warn("Some of the configuration could not be restored.")
    
    device.save_configuration()
    logger.info("Configuration restored.")
