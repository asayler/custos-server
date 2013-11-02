#!/usr/bin/env python

import shelve
from contextlib import closing

_DB_KEYS = "keys"

if __name__ == "__main__":
    with closing(shelve.open(_DB_KEYS, 'c')) as keys:
        pass
