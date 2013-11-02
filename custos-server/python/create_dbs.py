#!/usr/bin/env python

import shelve
from contextlib import closing

_DB_KEYS = "keys"

_UUID1 = "1b4e28ba-2fa1-11d2-883f-b9a761bde3fb"
_VAL1  = "VGhpcyBpcyBhIHNlY3JldCBrZXkhAA==" #"This is a secret key!"

if __name__ == "__main__":
    with closing(shelve.open(_DB_KEYS, 'c')) as keys:
        keys[_UUID1] = _VAL1
