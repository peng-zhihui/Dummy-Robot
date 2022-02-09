
import sys
import socket
import time
import traceback
import fibre.protocol
from fibre.utils import wait_any

def noprint(x):
  pass

class UDPTransport(fibre.protocol.PacketSource, fibre.protocol.PacketSink):
  def __init__(self, dest_addr, dest_port, logger):
    # TODO: FIXME: use IPv6
    # Problem: getaddrinfo fails if the resolver returns an
    # IPv4 address, but we are using AF_INET6
    #family = socket.AF_INET6 if socket.has_ipv6 else socket.AF_INET
    family = socket.AF_INET
    self.sock = socket.socket(family, socket.SOCK_DGRAM)
    # TODO: Determine the right address to use from the list
    self.target = socket.getaddrinfo(dest_addr,dest_port, family)[0][4]

  def process_packet(self, buffer):
    self.sock.sendto(buffer, self.target)

  def get_packet(self, deadline):
    # TODO: implement deadline
    deadline = None if deadline is None else max(deadline - time.monotonic(), 0)
    self.sock.settimeout(deadline)
    try:
      data, _ = self.sock.recvfrom(1024)
      return data
    except socket.timeout:
      # if we got a timeout data will still be none, so we call recv again
      # this time in non blocking state and see if we can get some data
      try:
        return self.sock.recvfrom(1024)
      except socket.timeout:
        raise TimeoutError

def discover_channels(path, serial_number, callback, cancellation_token, channel_termination_token, logger):
  """
  Tries to connect to a UDP server based on the path spec.
  This function blocks until cancellation_token is set.
  Channels spawned by this function run until channel_termination_token is set.
  """
  try:
    dest_addr = ':'.join(path.split(":")[:-1])
    dest_port = int(path.split(":")[-1])
  except (ValueError, IndexError):
    raise Exception('"{}" is not a valid UDP destination. The format should be something like "localhost:1234".'
                    .format(path))

  while not cancellation_token.is_set():
    try:
      udp_transport = fibre.udp_transport.UDPTransport(dest_addr, dest_port, logger)
      channel = fibre.protocol.Channel(
              "UDP device {}:{}".format(dest_addr, dest_port),
              udp_transport, udp_transport,
              channel_termination_token, logger)
    except:
      logger.debug("UDP channel init failed. More info: " + traceback.format_exc())
      pass
    else:
      callback(channel)
      wait_any(None, cancellation_token, channel._channel_broken)
    time.sleep(1)
