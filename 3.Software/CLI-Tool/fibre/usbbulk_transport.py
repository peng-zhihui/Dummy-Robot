# requires pyusb
#   pip install --pre pyusb

import usb.core
import usb.util
import sys
import time
import fibre.protocol
import traceback
import platform
from fibre.utils import TimeoutError

# Currently we identify fibre-enabled devices by VID,PID
# TODO: identify by USB descriptors
WELL_KNOWN_VID_PID_PAIRS = [
  (0x1209, 0x0D31),
  (0x1209, 0x0D32),
  (0x1209, 0x0D33)
]

class USBBulkTransport(fibre.protocol.PacketSource, fibre.protocol.PacketSink):
  def __init__(self, dev, logger):
    self._logger = logger
    self.dev = dev
    self.intf = None
    self._name = "USB device {}:{}".format(dev.idVendor, dev.idProduct)
    self._was_damaged = False

  ##
  # information about the connected device
  ##
  def info(self):
    # loop through configurations
    string = ""
    for cfg in self.dev:
      string += "ConfigurationValue {0}\n".format(cfg.bConfigurationValue)
      for intf in cfg:
        string += "\tInterfaceNumber {0},{1}\n".format(intf.bInterfaceNumber, intf.bAlternateSetting)
        for ep in intf:
          string += "\t\tEndpointAddress {0}\n".format(ep.bEndpointAddress)
    return string

  def init(self):
    # Under some conditions, the Linux USB/libusb stack ends up in a corrupt
    # state where there are a few packets in a receive queue but a call
    # to epr.read() does not return these packet until a new packet arrives.
    # This undesirable queue can be cleared by resetting the device.
    # On windows this would cause file-not-found errors in subsequent dev calls
    if platform.system() != 'Windows':
        self.dev.reset()

    #self.dev.set_configuration() # no args: set first configuration

    # Find the best interface
    self.cfg = self.dev.get_active_configuration()
    custom_interfaces = [i for i in self.cfg.interfaces() if i.bInterfaceClass == 0x00 and i.bInterfaceSubClass == 0x01]
    cdc_interfaces = [i for i in self.cfg.interfaces() if i.bInterfaceClass == 0x0a and i.bInterfaceSubClass == 0x00]
    all_compatible_interfaces = custom_interfaces + cdc_interfaces
    if len(all_compatible_interfaces) == 0:
      raise Exception("the device has no compatible interfaces")
    self.intf = all_compatible_interfaces[0]

    # Try to detach kernel driver from interface
    try:
      if self.dev.is_kernel_driver_active(self.intf.bInterfaceNumber):
        self.dev.detach_kernel_driver(self.intf.bInterfaceNumber)
        self._logger.debug("Detached Kernel Driver")
      else:
        self._logger.debug("Kernel Driver was not attached")
    except NotImplementedError:
      pass #is_kernel_driver_active not implemented on Windows

    # find write endpoint (first OUT endpoint)
    self.epw = usb.util.find_descriptor(self.intf,
        custom_match = \
        lambda e: \
            usb.util.endpoint_direction(e.bEndpointAddress) == \
            usb.util.ENDPOINT_OUT
    )
    assert self.epw is not None
    self._logger.debug("EndpointAddress for writing {}".format(self.epw.bEndpointAddress))
    # find read endpoint (first IN endpoint)
    self.epr = usb.util.find_descriptor(self.intf,
        custom_match = \
        lambda e: \
            usb.util.endpoint_direction(e.bEndpointAddress) == \
            usb.util.ENDPOINT_IN
    )
    assert self.epr is not None
    self._logger.debug("EndpointAddress for reading {}".format(self.epr.bEndpointAddress))

  def deinit(self):
    if not self.intf is None:
      usb.util.release_interface(self.dev, self.intf)

  def process_packet(self, usbBuffer):
    try:
      ret = self.epw.write(usbBuffer, 0)
      if self._was_damaged:
        self._logger.debug("Recovered from USB halt/stall condition")
        self._was_damaged = False
      return ret
    except usb.core.USBError as ex:
      if ex.errno == 19 or ex.errno == 32: # "no such device", "pipe error"
        raise fibre.protocol.ChannelBrokenException()
      elif ex.errno is None or ex.errno == 60 or ex.errno == 110: # timeout
        raise TimeoutError()
      else:
        self._logger.debug("error in usbbulk_transport.py, process_packet")
        self._logger.debug(traceback.format_exc())
        self._logger.debug("halt condition: {}".format(ex.errno))
        self._logger.debug(str(ex))
        # Try resetting halt/stall condition
        try:
          self.deinit()
          self.init()
        except usb.core.USBError:
          raise fibre.protocol.ChannelBrokenException()
        # Retry transfer
        self._was_damaged = True
        raise fibre.protocol.ChannelDamagedException()

  def get_packet(self, deadline):
    try:
      bufferLen = self.epr.wMaxPacketSize
      timeout = max(int((deadline - time.monotonic()) * 1000), 0)
      ret = self.epr.read(bufferLen, timeout)
      if self._was_damaged:
        self._logger.debug("Recovered from USB halt/stall condition")
        self._was_damaged = False
      return bytearray(ret)
    except usb.core.USBError as ex:
      if ex.errno == 19 or ex.errno == 32: # "no such device", "pipe error"
        raise fibre.protocol.ChannelBrokenException()
      elif ex.errno is None or ex.errno == 60 or ex.errno == 110: # timeout
        raise TimeoutError()
      else:
        self._logger.debug("error in usbbulk_transport.py, process_packet")
        self._logger.debug(traceback.format_exc())
        self._logger.debug("halt condition: {}".format(ex.errno))
        self._logger.debug(str(ex))
        # Try resetting halt/stall condition
        try:
          self.deinit()
          self.init()
        except usb.core.USBError:
          raise fibre.protocol.ChannelBrokenException()
        # Retry transfer
        self._was_damaged = True
        raise fibre.protocol.ChannelDamagedException()


def discover_channels(path, serial_number, callback, cancellation_token, channel_termination_token, logger):
  """
  Scans for USB devices that match the path spec.
  This function blocks until cancellation_token is set.
  Channels spawned by this function run until channel_termination_token is set.
  """
  if path == None or path == "":
    bus = None
    address = None
  else:
    try:
      bus = int(path.split(":")[0])
      address = int(path.split(":")[1])
    except (ValueError, IndexError):
      raise Exception("{} is not a valid USB path specification. "
                      "Expected a string of the format BUS:DEVICE where BUS "
                      "and DEVICE are integers.".format(path))
  
  known_devices = []
  def device_matcher(device):
    #print("  test {:04X}:{:04X}".format(device.idVendor, device.idProduct))
    try:
      if (device.bus, device.address) in known_devices:
        return False
      if bus != None and device.bus != bus:
        return False
      if address != None and device.address != address:
        return False
      if serial_number != None and device.serial_number != serial_number:
        return False
      if (device.idVendor, device.idProduct) not in WELL_KNOWN_VID_PID_PAIRS:
        return False
    except:
      return False
    return True

  while not cancellation_token.is_set():
    logger.debug("USB discover loop")
    devices = usb.core.find(find_all=True, custom_match=device_matcher)
    for usb_device in devices:
      try:
        bulk_device = USBBulkTransport(usb_device, logger)
        logger.debug(bulk_device.info())
        bulk_device.init()
        channel = fibre.protocol.Channel(
                "USB device bus {} device {}".format(usb_device.bus, usb_device.address),
                bulk_device, bulk_device, channel_termination_token, logger)
        channel.usb_device = usb_device # for debugging only
      except usb.core.USBError as ex:
        if ex.errno == 13:
          logger.debug("USB device access denied. Did you set up your udev rules correctly?")
          continue
        elif ex.errno == 16:
          logger.debug("USB device busy. I'll reset it and try again.")
          usb_device.reset()
          continue
        else:
          logger.debug("USB device init failed. Ignoring this device. More info: " + traceback.format_exc())
          known_devices.append((usb_device.bus, usb_device.address))
      else:
        known_devices.append((usb_device.bus, usb_device.address))
        callback(channel)
    time.sleep(1)
