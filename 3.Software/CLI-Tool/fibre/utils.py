
import sys
import time
import threading
import platform
import subprocess
import os

try:
    if platform.system() == 'Windows':
        import win32console
        # TODO: we should win32console anyway so we could just omit colorama
        import colorama
        colorama.init()
except ImportError:
    print("Could not init terminal features.")
    sys.stdout.flush()
    pass
    
if sys.version_info < (3, 3):
    class TimeoutError(Exception):
        pass
else:
    TimeoutError = TimeoutError

def get_serial_number_str(device):
    if hasattr(device, 'serial_number'):
        return format(device.serial_number, 'x').upper()
    else:
        return "[unknown serial number]"

## Threading utils ##
class Event():
    """
    Alternative to threading.Event(), enhanced by the subscribe() function
    that the original fails to provide.
    @param Trigger: if supplied, the newly created event will be triggered
                    as soon as the trigger event becomes set
    """
    def __init__(self, trigger=None):
        self._evt = threading.Event()
        self._subscribers = []
        self._mutex = threading.Lock()
        if not trigger is None:
            trigger.subscribe(lambda: self.set())

    def is_set(self):
        return self._evt.is_set()

    def set(self):
        """
        Sets the event and invokes all subscribers if the event was
        not already set
        """
        self._mutex.acquire()
        try:
            if not self._evt.is_set():
                self._evt.set()
                for s in self._subscribers:
                    s()
        finally:
            self._mutex.release()

    def subscribe(self, handler):
        """
        Invokes the specified handler exactly once as soon as the
        specified event is set. If the event is already set, the
        handler is invoked immediately.
        Returns a function that can be invoked to unsubscribe.
        """
        if handler is None:
            raise TypeError
        self._mutex.acquire()
        try:
            self._subscribers.append(handler)
            if self._evt.is_set():
                handler()
        finally:
            self._mutex.release()
        return handler
    
    def unsubscribe(self, handler):
        self._mutex.acquire()
        try:
            self._subscribers.pop(self._subscribers.index(handler))
        finally:
            self._mutex.release()

    def wait(self, timeout=None):
        if not self._evt.wait(timeout=timeout):
            raise TimeoutError()

    def trigger_after(self, timeout):
        """
        Triggers the event after the specified timeout.
        This function returns immediately.
        """
        def delayed_trigger():
            if not self.wait(timeout=timeout):
                self.set()
        threading.Thread(target=delayed_trigger)
        t.daemon = True
        t.start()
        

def wait_any(timeout=None, *events):
    """
    Blocks until any of the specified events are triggered.
    Returns the index of the event that was triggerd or raises
    a TimeoutError
    Param timeout: A timeout in seconds
    """
    or_event = threading.Event()
    subscriptions = []
    for event in events:
        subscriptions.append((event, event.subscribe(lambda: or_event.set())))
    or_event.wait(timeout=timeout)
    for event, sub in subscriptions:
        event.unsubscribe(sub)
    for i in range(len(events)):
        if events[i].is_set():
            return i
    raise TimeoutError()


## Log utils ##

class Logger():
    """
    Logs messages to stdout
    """

    COLOR_DEFAULT = 0
    COLOR_GREEN = 1
    COLOR_CYAN = 2
    COLOR_YELLOW = 3
    COLOR_RED = 4

    _VT100Colors = {
        COLOR_GREEN: '\x1b[92;1m',
        COLOR_CYAN: '\x1b[96;1m',
        COLOR_YELLOW: '\x1b[93;1m',
        COLOR_RED: '\x1b[91;1m',
        COLOR_DEFAULT: '\x1b[0m'
    }

    _Win32Colors = {
        COLOR_GREEN: 0x0A,
        COLOR_CYAN: 0x0B,
        COLOR_YELLOW: 0x0E,
        COLOR_RED: 0x0C,
        COLOR_DEFAULT: 0x07
    }

    def __init__(self, verbose=True):
        self._prefix = ''
        self._skip_bottom_line = False # If true, messages are printed one line above the cursor
        self._verbose = verbose
        self._print_lock = threading.Lock()
        if platform.system() == 'Windows':
            self._stdout_buf = win32console.GetStdHandle(win32console.STD_OUTPUT_HANDLE)

    def indent(self, prefix='  '):
        indented_logger = Logger()
        indented_logger._prefix = self._prefix + prefix
        return indented_logger

    def print_on_second_last_line(self, text, color):
        """
        Prints a text on the second last line.
        This can be used to print a message above the command
        prompt. If the command prompt spans multiple lines
        there will be glitches.
        If the printed text spans multiple lines there will also
        be glitches (though this could be fixed).
        """

        if platform.system() == 'Windows':
            # Windows <10 doesn't understand VT100 escape codes and the colorama
            # also doesn't support the specific escape codes we need so we use the
            # native Win32 API.
            info = self._stdout_buf.GetConsoleScreenBufferInfo()
            cursor_pos = info['CursorPosition']
            scroll_rect=win32console.PySMALL_RECTType(
                Left=0, Top=1,
                Right=info['Window'].Right,
                Bottom=cursor_pos.Y-1)
            scroll_dest = win32console.PyCOORDType(scroll_rect.Left, scroll_rect.Top-1)
            self._stdout_buf.ScrollConsoleScreenBuffer(
                scroll_rect, scroll_rect, scroll_dest, # clipping rect is same as scroll rect
                u' ', Logger._Win32Colors[color]) # fill with empty cells with the desired color attributes
            line_start = win32console.PyCOORDType(0, cursor_pos.Y-1)
            self._stdout_buf.WriteConsoleOutputCharacter(text, line_start)

        else:
            # Assume we're in a terminal that interprets VT100 escape codes.
            # TODO: test on macOS

            # Escape character sequence:
            #   ESC 7: store cursor position
            #   ESC 1A: move cursor up by one
            #   ESC 1S: scroll entire viewport by one
            #   ESC 1L: insert 1 line at cursor position
            #   (print text)
            #   ESC 8: restore old cursor position

            self._print_lock.acquire()
            sys.stdout.write('\x1b7\x1b[1A\x1b[1S\x1b[1L')
            sys.stdout.write(Logger._VT100Colors[color] + text + Logger._VT100Colors[Logger.COLOR_DEFAULT])
            sys.stdout.write('\x1b8')
            sys.stdout.flush()
            self._print_lock.release()

    def print_colored(self, text, color):
        if self._skip_bottom_line:
            self.print_on_second_last_line(text, color)
        else:
            # On Windows, colorama does the job of interpreting the VT100 escape sequences
            self._print_lock.acquire()
            sys.stdout.write(Logger._VT100Colors[color] + text + Logger._VT100Colors[Logger.COLOR_DEFAULT] + '\n')
            sys.stdout.flush()
            self._print_lock.release()

    def debug(self, text):
        if self._verbose:
            self.print_colored(self._prefix + text, Logger.COLOR_DEFAULT)
    def success(self, text):
        self.print_colored(self._prefix + text, Logger.COLOR_GREEN)
    def info(self, text):
        self.print_colored(self._prefix + text, Logger.COLOR_DEFAULT)
    def notify(self, text):
        self.print_colored(self._prefix + text, Logger.COLOR_CYAN)
    def warn(self, text):
        self.print_colored(self._prefix + text, Logger.COLOR_YELLOW)
    def error(self, text):
        # TODO: write to stderr
        self.print_colored(self._prefix + text, Logger.COLOR_RED)
