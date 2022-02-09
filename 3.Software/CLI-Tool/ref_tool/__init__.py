import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(
    os.path.dirname(os.path.realpath(__file__)))),
    "Firmware", "fibre", "python"))

# Syntactic sugar to make usage more intuative.
# Try/pass used to break install-time dep issues
try:
    import fibre

    find_any = fibre.find_any
    find_all = fibre.find_all
except:
    pass

# Standard convention is to add a __version__ attribute to the package
from .version import get_version_str

del get_version_str
