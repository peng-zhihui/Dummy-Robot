"""
Provides functions for the discovery of Fibre nodes
"""

import sys
import json
import struct
import threading
import fibre.protocol

class ObjectDefinitionError(Exception):
    pass

codecs = {}

class StructCodec():
    """
    Generic serializer/deserializer based on struct pack
    """
    def __init__(self, struct_format, target_type):
        self._struct_format = struct_format
        self._target_type = target_type
    def get_length(self):
        return struct.calcsize(self._struct_format)
    def serialize(self, value):
        value = self._target_type(value)
        return struct.pack(self._struct_format, value)
    def deserialize(self, buffer):
        value = struct.unpack(self._struct_format, buffer)
        value = value[0] if len(value) == 1 else value
        return self._target_type(value)

class RemoteProperty():
    """
    Used internally by dynamically created objects to translate
    property assignments and fetches into endpoint operations on the
    object's associated channel
    """
    def __init__(self, json_data, parent):
        self._parent = parent
        self.__channel__ = parent.__channel__
        id_str = json_data.get("id", None)
        if id_str is None:
            raise ObjectDefinitionError("unspecified endpoint ID")
        self._id = int(id_str)

        self._name = json_data.get("name", None)
        if self._name is None:
            self._name = "[anonymous]"

        type_str = json_data.get("type", None)
        if type_str is None:
            raise ObjectDefinitionError("unspecified type")

        # Find all codecs that match the type_str and build a dictionary
        # of the form {type1: codec1, type2: codec2}
        eligible_types = {k: v[type_str] for (k,v) in codecs.items() if type_str in v}
        
        if not eligible_types:
            raise ObjectDefinitionError("unsupported codec {}".format(type_str))

        # TODO: better heuristics to select a matching type (i.e. prefer non lossless)
        eligible_types = list(eligible_types.items())
        self._property_type = eligible_types[0][0]
        self._codec = eligible_types[0][1]

        access_mode = json_data.get("access", "r")
        self._can_read = 'r' in access_mode
        self._can_write = 'w' in access_mode

    def get_value(self):
        buffer = self._parent.__channel__.remote_endpoint_operation(self._id, None, True, self._codec.get_length())
        return self._codec.deserialize(buffer)

    def set_value(self, value):
        buffer = self._codec.serialize(value)
        # TODO: Currenly we wait for an ack here. Settle on the default guarantee.
        self._parent.__channel__.remote_endpoint_operation(self._id, buffer, True, 0)

    def _dump(self):
        if self._name == "serial_number":
            # special case: serial number should be displayed in hex (TODO: generalize)
            val_str = "{:012X}".format(self.get_value())
        elif self._name == "error":
            # special case: errors should be displayed in hex (TODO: generalize)
            val_str = "0x{:04X}".format(self.get_value())
        else:
            val_str = str(self.get_value())
        return "{} = {} ({})".format(self._name, val_str, self._property_type.__name__)

class EndpointRefCodec():
    """
    Serializer/deserializer for an endpoint reference
    """
    def get_length(self):
        return struct.calcsize("<HH")
    def serialize(self, value):
        if value is None:
            (ep_id, ep_crc) = (0, 0)
        elif isinstance(value, RemoteProperty):
            (ep_id, ep_crc) = (value._id, value.__channel__._interface_definition_crc)
        else:
            raise TypeError("Expected value of type RemoteProperty or None but got '{}'. En example for a RemoteProperty is this expression: odrv0.axis0.controller._remote_attributes['pos_setpoint']".format(type(value).__name__))
        return struct.pack("<HH", ep_id, ep_crc)
    def deserialize(self, buffer):
        return struct.unpack("<HH", buffer)

codecs[int] = {
    'int8': StructCodec("<b", int),
    'uint8': StructCodec("<B", int),
    'int16': StructCodec("<h", int),
    'uint16': StructCodec("<H", int),
    'int32': StructCodec("<i", int),
    'uint32': StructCodec("<I", int),
    'int64': StructCodec("<q", int),
    'uint64': StructCodec("<Q", int)
}

codecs[bool] = {
    'bool': StructCodec("<?", bool)
}

codecs[float] = {
    'float': StructCodec("<f", float)
}

codecs[RemoteProperty] = {
    'endpoint_ref': EndpointRefCodec()
}


class RemoteFunction(object):
    """
    Represents a callable function that maps to a function call on a remote object
    """
    def __init__(self, json_data, parent):
        self._parent = parent
        id_str = json_data.get("id", None)
        if id_str is None:
            raise ObjectDefinitionError("unspecified endpoint ID")
        self._trigger_id = int(id_str)

        self._name = json_data.get("name", None)
        if self._name is None:
            self._name = "[anonymous]"

        self._inputs = []
        for param_json in json_data.get("arguments", []) + json_data.get("inputs", []): # TODO: deprecate "arguments" keyword
            param_json["mode"] = "r"
            self._inputs.append(RemoteProperty(param_json, parent))

        self._outputs = []
        for param_json in json_data.get("outputs", []): # TODO: deprecate "arguments" keyword
            param_json["mode"] = "r"
            self._outputs.append(RemoteProperty(param_json, parent))

    def __call__(self, *args):
        if (len(self._inputs) != len(args)):
            raise TypeError("expected {} arguments but have {}".format(len(self._inputs), len(args)))
        for i in range(len(args)):
            self._inputs[i].set_value(args[i])
        self._parent.__channel__.remote_endpoint_operation(self._trigger_id, None, True, 0)
        if len(self._outputs) > 0:
            return self._outputs[0].get_value()

    def _dump(self):
        return "{}({})".format(self._name, ", ".join("{}: {}".format(x._name, x._property_type.__name__) for x in self._inputs))

class RemoteObject(object):
    """
    Object with functions and properties that map to remote endpoints
    """
    def __init__(self, json_data, parent, channel, logger):
        """
        Creates an object that implements the specified JSON type description by
        communicating over the provided channel
        """
        # Directly write to __dict__ to avoid calling __setattr__ too early
        object.__getattribute__(self, "__dict__")["_remote_attributes"] = {}
        object.__getattribute__(self, "__dict__")["__sealed__"] = False
        # Assign once more to make linter happy
        self._remote_attributes = {}
        self.__sealed__ = False

        self.__channel__ = channel
        self.__parent__ = parent

        # Build attribute list from JSON
        for member_json in json_data.get("members", []):
            member_name = member_json.get("name", None)
            if member_name is None:
                logger.debug("ignoring unnamed attribute")
                continue

            try:
                type_str = member_json.get("type", None)
                if type_str == "object":
                    attribute = RemoteObject(member_json, self, channel, logger)
                elif type_str == "function":
                    attribute = RemoteFunction(member_json, self)
                elif type_str != None:
                    attribute = RemoteProperty(member_json, self)
                else:
                    raise ObjectDefinitionError("no type information")
            except ObjectDefinitionError as ex:
                logger.debug("malformed member {}: {}".format(member_name, str(ex)))
                continue

            self._remote_attributes[member_name] = attribute
            self.__dict__[member_name] = attribute

        # Ensure that from here on out assignments to undefined attributes
        # raise an exception
        self.__sealed__ = True
        channel._channel_broken.subscribe(self._tear_down)

    def _dump(self, indent, depth):
        if depth <= 0:
            return "..."
        lines = []
        for key, val in self._remote_attributes.items():
            if isinstance(val, RemoteObject):
                val_str = indent + key + (": " if depth == 1 else ":\n") + val._dump(indent + "  ", depth - 1)
            else:
                val_str = indent + val._dump()
            lines.append(val_str)
        return "\n".join(lines)

    def __str__(self):
        return self._dump("", depth=2)

    def __repr__(self):
        return self.__str__()

    def __getattribute__(self, name):
        attr = object.__getattribute__(self, "_remote_attributes").get(name, None)
        if isinstance(attr, RemoteProperty):
            if attr._can_read:
                return attr.get_value()
            else:
                raise Exception("Cannot read from property {}".format(name))
        elif attr != None:
            return attr
        else:
            return object.__getattribute__(self, name)
            #raise AttributeError("Attribute {} not found".format(name))

    def __setattr__(self, name, value):
        attr = object.__getattribute__(self, "_remote_attributes").get(name, None)
        if isinstance(attr, RemoteProperty):
            if attr._can_write:
                attr.set_value(value)
            else:
                raise Exception("Cannot write to property {}".format(name))
        elif not object.__getattribute__(self, "__sealed__") or name in object.__getattribute__(self, "__dict__"):
            object.__getattribute__(self, "__dict__")[name] = value
        else:
            raise AttributeError("Attribute {} not found".format(name))

    def _tear_down(self):
        # Clear all remote members
        for k in self._remote_attributes.keys():
            self.__dict__.pop(k)
        self._remote_attributes = {}
