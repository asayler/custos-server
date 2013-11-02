#!/usr/bin/env python

import shelve
from contextlib import closing

_DB_KEYS = "keys"
_DB_ATTRS = "attrs"
_DB_ACLS_READ = "acls_read"

_ATTR_UUID1 = "9b386945-955e-4b7a-9668-641786843357"
_ATTR_VAL1 = { "Class": "explicit",
               "Type": "psk",
               "Index": 0,
               "Value": "SXQncyBBIFRyYXAhAA==" }

_KEY_UUID1 = "1b4e28ba-2fa1-11d2-883f-b9a761bde3fb"
_KEY_VAL1  = "VGhpcyBpcyBhIHNlY3JldCBrZXkhAA=="
_ACLS_READ1 = [[_ATTR_UUID1]]

if __name__ == "__main__":
    with closing(shelve.open(_DB_KEYS, 'c')) as keys:
        keys[_KEY_UUID1] = _KEY_VAL1
    with closing(shelve.open(_DB_ATTRS, 'c')) as attrs:
        attrs[_ATTR_UUID1] = _ATTR_VAL1
    with closing(shelve.open(_DB_ACLS_READ, 'c')) as acls_read:
        acls_read[_KEY_UUID1] = _ACLS_READ1
