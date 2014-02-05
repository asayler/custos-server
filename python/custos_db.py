#!/usr/bin/env python

import shelve
from contextlib import closing

_SRV_UUID = "30976aa2-fcf9-463e-a6ed-ba7e3ef6ebd4"

_DB_SRV_ACS = "srv_acs"
_DB_GRP_ACS = "grp_acs"
_DB_OBJ_ACS = "obj_acs"

_DB_SRV_GRPS = "srv_grps"

_DB_KEYS = "keys"
_DB_ATTRS = "attrs"
_DB_ACLS_READ = "acls_read"



_ATTR_UUID_1 = "9b386945-955e-4b7a-9668-641786843357"
_ATTR_VAL_1 = { "Class": "explicit",
                "Type": "psk",
                "Index": 0,
                "Value": "SXQncyBBIFRyYXAhAA==" }

_ACL_1 = [[_ATTR_UUID_1]]

_SRV_ACS_1 = { "srv_grp_create": _ACL_1,
               "srv_grp_list": _ACL_1,
               "srv_grp_ovr": _ACL_1,
               "srv_audit": _ACL_1,
               "srv_clean": _ACL_1,
               "srv_acs_get": _ACL_1,
               "srv_acs_set": _ACL_1 }

_GRP_UUID_1 = "ce7ec850-fa77-4aa8-8014-8abf04b077d5"
_GRP_ACS_1 = { "grp_obj_create": _ACL_1,
               "grp_obj_list": _ACL_1,
               "grp_obj_ovr": _ACL_1,
               "grp_delete": _ACL_1,
               "grp_audit": _ACL_1,
               "grp_clean": _ACL_1,
               "grp_acs_get": _ACL_1,
               "grp_acs_set": _ACL_1 }

_SRV_GRPS_1 = [ _GRP_UUID_1 ]

_KEY_UUID_1 = "1b4e28ba-2fa1-11d2-883f-b9a761bde3fb"
_KEY_VAL_1  = "VGhpcyBpcyBhIHNlY3JldCBrZXkhAA=="


def get_srv_grps():

    with closing(shelve.open(_DB_SRV_GRPS, 'r')) as srv_grps:
        if _SRV_UUID in srv_grps:
            return srv_grps[_SRV_UUID]
        else:
            return None

def get_srv_ACS():

    with closing(shelve.open(_DB_SRV_ACS, 'r')) as srv_acs:
        if _SRV_UUID in srv_acs:
            return srv_acs[_SRV_UUID]
        else:
            return None

def get_grp_ACS(uuid):

    with closing(shelve.open(_DB_GRP_ACS, 'r')) as grp_acs:
        if uuid in grp_acs:
            return grp_acs[uuid]
        else:
            return None

def get_obj_ACS(uuid):

    with closing(shelve.open(_DB_OBJ_ACS, 'r')) as obj_acs:
        if uuid in obj_acs:
            return obj_acs[uuid]
        else:
            return None


def get_key_val(uuid):

    with closing(shelve.open(_DB_KEYS, 'r')) as keys:
        if uuid in keys:
            return keys[uuid]
        else:
            return None

def get_attr_val(uuid):

    with closing(shelve.open(_DB_ATTRS, 'r')) as attrs:
        if uuid in attrs:
            return attrs[uuid]
        else:
            return None

def get_ACLS_read(uuid):

    with closing(shelve.open(_DB_ACLS_READ, 'r')) as acls:
        if uuid in acls:
            return acls[uuid]
        else:
            return None

if __name__ == "__main__":

    with closing(shelve.open(_DB_SRV_ACS, 'c')) as srv_acs:
        srv_acs[_SRV_UUID] = _SRV_ACS_1
    with closing(shelve.open(_DB_SRV_GRPS, 'c')) as srv_grps:
        srv_grps[_SRV_UUID] = _SRV_GRPS_1
    with closing(shelve.open(_DB_GRP_ACS, 'c')) as grp_acs:
        grp_acs[_GRP_UUID_1] = _GRP_ACS_1
    with closing(shelve.open(_DB_KEYS, 'c')) as keys:
        keys[_KEY_UUID_1] = _KEY_VAL_1
    with closing(shelve.open(_DB_ATTRS, 'c')) as attrs:
        attrs[_ATTR_UUID_1] = _ATTR_VAL_1
    with closing(shelve.open(_DB_ACLS_READ, 'c')) as acls_read:
        acls_read[_KEY_UUID_1] = _ACL_1
