import iso_read as iso
import sys

# just use the iso reader
class Parser(iso.Parser):
    def __init__(self):
        iso.Parser.__init__(self)
