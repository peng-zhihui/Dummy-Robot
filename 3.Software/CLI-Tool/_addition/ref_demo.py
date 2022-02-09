#!/usr/bin/env python3

from __future__ import print_function
import ref_tool

# Find a connected REF-Unit (this will block until you connect one)
print("finding an ref_tool...")
my_drive = ref_tool.find_any()

# Find an REF-Unit that is connected on the serial port /dev/ttyUSB0
# my_drive = ref_tool.find_any("serial:/dev/ttyUSB0")
