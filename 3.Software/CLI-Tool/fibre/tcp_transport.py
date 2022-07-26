
import sys
import socket
import time
import traceback
import fibre.protocol
from fibre.utils import wait_any, TimeoutError

def noprint(x):
  pass

class TCPTransport(fibre.protocol.StreamSource, fibre.protocol.StreamSink):
  def __init__(self, dest_addr, dest_port, logger):
    # TODO: FIXME: use IPv6
    # Problem: getaddrinfo fails if the resolver returns an
    # IPv4 address, but we are using AF_INET6
    #family = socket.AF_INET6 if socket.has_ipv6 else socket.AF_INET
    family = socket.AF_INET
    self.sock = socket.socket(family, socket.SOCK_STREAM)
    # TODO: Determine the right address to use from the list
    self.target = socket.getaddrinfo(dest_addr, dest_port, family)[0][4]
    # TODO: this blocks until a connection is established, or the system cancels it
    self.sock.connect(self.target)

  def process_bytes(self, buffer):
    self.sock.send(buffer)

  def get_bytes(self, n_bytes, deadline):
    """
    Returns n bytes unless the deadline is reached, in which case the bytes
    that were read up to that point are returned. If deadline is None the
    function blocks forever. A deadline before the current time corresponds
    to non-blocking mode.
    """
    # convert deadline to seconds (floating point)
    deadline = None if deadline is None else max(deadline - time.monotonic(), 0)
    self.sock.settimeout(deadline)
    try:
      data = self.sock.recv(n_bytes) # receive n_bytes
      return data
    except socket.timeout:
      # if we got a timeout data will still be none, so we call recv again
      # this time in non blocking state and see if we can get some data
      try:
        return self.sock.recv(n_bytes)
      except socket.timeout:
        raise TimeoutError

  def get_bytes_or_fail(self, n_bytes, deadline):
    result = self.get_bytes(n_bytes, deadline)
    if len(result) < n_bytes:
      raise TimeoutError("expected {} bytes but got only {}".format(n_bytes, len(result)))
    return result



def discover_channels(path, serial_number, callback, cancellation_token, channel_termination_token, logger):
  """
  Tries to connect to a TCP server based on the path spec.
  This function blocks until cancellation_token is set.
  Channels spawned by this function run until channel_termination_token is set.
  """
  try:
    dest_addr = ':'.join(path.split(":")[:-1])
    dest_port = int(path.split(":")[-1])
  except (ValueError, IndexError):
    raise Exception('"{}" is not a valid TCP destination. The format should be something like "localhost:1234".'
                    .format(path))

  while not cancellation_token.is_set():
    try:
      tcp_transport = fibre.tcp_transport.TCPTransport(dest_addr, dest_port, logger)
      stream2packet_input = fibre.protocol.PacketFromStreamConverter(tcp_transport)
      packet2stream_output = fibre.protocol.StreamBasedPacketSink(tcp_transport)
      channel = fibre.protocol.Channel(
              "TCP device {}:{}".format(dest_addr, dest_port),
              stream2packet_input, packet2stream_output,
              channel_termination_token, logger)
    except:
      #logger.debug("TCP channel init failed. More info: " + traceback.format_exc())
      pass
    else:
      callback(channel)
      wait_any(None, cancellation_token, channel._channel_broken)
    time.sleep(1)
