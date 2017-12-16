"""@package module
Parent class parses the command line arguments and makes every base class extend handle_args
method to do the work
"""

import abc


class Parent(object):
    """Parent class parses and has an abstract argument handler"""

    class Colors:
        """Colors for the console"""

        HEADER = '\033[95m'
        OKBLUE = '\033[94m'
        OKGREEN = '\033[92m'
        WARNING = '\033[93m'
        FAIL = '\033[91m'
        ENDC = '\033[0m'
        BOLD = '\033[1m'
        UNDERLINE = '\033[4m'

    __metaclass__ = abc.ABCMeta

    def parse(self):
        """Parse command line arguments"""

        from sys import argv
        args = {}  # Empty dictionary to store key-value pairs.
        while argv:  # While there are arguments left to parse...
            if argv[0][0] == '-':  # Found a "-name value" pair.
                if len(argv) == 1:
                    args[argv[0]] = ""
                else:
                    args[argv[0]] = argv[1]  # Add key and value to the dictionary.
            argv = argv[1:]  # Reduce the argument list by copying it starting from index 1.
        return args

    @abc.abstractmethod
    def handle_args(self, args):
        """Handle command line arguments"""
        return

    def start(self):
        self.handle_args(self.parse())
