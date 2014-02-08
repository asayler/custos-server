#!/usr/bin/env python
# -*- coding: utf-8 -*-

import shelve
from contextlib import closing

_ENCODING = 'utf-8'
_VER_SEPERATOR = '+'

_DB_SRV_ACS = "db_srv_acs"
_DB_GRP_ACS = "db_grp_acs"
_DB_OBJ_ACS = "db_obj_acs"

_DB_SRV_GRPS = "db_srv_grps"
_DB_GRP_OBJS = "db_grp_objs"
_DB_OBJ_VAL = "db_obj_val"
_DB_OBJ_VER = "db_obj_ver"

_DB_AAS = "db_aas"

_AA_1_UUID = u"9b386945-955e-4b7a-9668-641786843357"
_AA_1_VAL = { u"Class": u"explicit",
              u"Type": u"psk",
              u"Value": u"SXQncyBBIFRyYXAhAA==" }

_ACL_1 = [ [ _AA_1_UUID ] ]

_OBJ_1_UUID = u"1b4e28ba-2fa1-11d2-883f-b9a761bde3fb"
_OBJ_1_VER = 1
_OBJ_1_ACS = { u"obj_delete": _ACL_1,
               u"obj_read": _ACL_1,
               u"obj_update": _ACL_1,
               u"obj_audit": _ACL_1,
               u"obj_clean": _ACL_1,
               u"obj_acs_get": _ACL_1,
               u"obj_acs_set": _ACL_1 }
_OBJ_1_VAL  = u"VGhpcyBpcyBhIHNlY3JldCBrZXkhAA=="

_GRP_1_UUID = u"ce7ec850-fa77-4aa8-8014-8abf04b077d5"
_GRP_1_ACS = { u"grp_obj_create": _ACL_1,
               u"grp_obj_list": _ACL_1,
               u"grp_obj_ovr": _ACL_1,
               u"grp_delete": _ACL_1,
               u"grp_audit": _ACL_1,
               u"grp_clean": _ACL_1,
               u"grp_acs_get": _ACL_1,
               u"grp_acs_set": _ACL_1 }
_GRP_1_OBJS = [ _OBJ_1_UUID ]

_SRV_1_UUID = u"30976aa2-fcf9-463e-a6ed-ba7e3ef6ebd4"
_SRV_1_ACS = { u"srv_grp_create": _ACL_1,
               u"srv_grp_list": _ACL_1,
               u"srv_grp_ovr": _ACL_1,
               u"srv_audit": _ACL_1,
               u"srv_clean": _ACL_1,
               u"srv_acs_get": _ACL_1,
               u"srv_acs_set": _ACL_1 }
_SRV_1_GRPS = [ _GRP_1_UUID ]

# Get ACS
def get_srv_ACS():

    with closing(shelve.open(_DB_SRV_ACS, 'r')) as srv_acs:
        if _SRV_1_UUID.encode(_ENCODING) in srv_acs:
            return srv_acs[_SRV_1_UUID.encode(_ENCODING)]
        else:
            return None

def get_grp_ACS(grp_uuid):

    with closing(shelve.open(_DB_GRP_ACS, 'r')) as grp_acs:
        if grp_uuid.encode(_ENCODING) in grp_acs:
            return grp_acs[grp_uuid.encode(_ENCODING)]
        else:
            return None

def get_obj_ACS(obj_uuid):

    with closing(shelve.open(_DB_OBJ_ACS, 'r')) as obj_acs:
        if obj_uuid.encode(_ENCODING) in obj_acs:
            return obj_acs[obj_uuid.encode(_ENCODING)]
        else:
            return None

# List OUs

def list_srv_grps():

    with closing(shelve.open(_DB_SRV_GRPS, 'r')) as srv_grps:
        if _SRV_1_UUID.encode(_ENCODING) in srv_grps:
            return srv_grps[_SRV_1_UUID.encode(_ENCODING)]
        else:
            return None

def list_grp_objs(grp_uuid):

    with closing(shelve.open(_DB_GRP_OBJS, 'r')) as grp_objs:
        if grp_uuid.encode(_ENCODING) in grp_objs:
            return grp_objs[grp_uuid.encode(_ENCODING)]
        else:
            return None

# Get OUs

def get_attr_val(aa_uuid, ver=None):

    with closing(shelve.open(_DB_AAS, 'r')) as aas:
        if aa_uuid.encode(_ENCODING) in aas:
            return aas[aa_uuid.encode(_ENCODING)]
        else:
            return None

def get_obj_val(obj_uuid, ver=None):

    if not ver:
        with closing(shelve.open(_DB_OBJ_VER, 'r')) as obj_ver:
            key_str = obj_uuid.encode(_ENCODING)
            if key_str in obj_ver:
                ver = obj_ver[key_str]
            else:
                return None

    with closing(shelve.open(_DB_OBJ_VAL, 'r')) as obj_val:
        key_str = build_uuid_ver(obj_uuid, ver).encode(_ENCODING)
        if key_str in obj_val:
            return obj_val[key_str]
        else:
            return None

def build_uuid_ver(uuid, ver):
    return uuid + _VER_SEPERATOR + unicode(ver)

# Main: Setup DBs

if __name__ == "__main__":

    with closing(shelve.open(_DB_AAS, 'c')) as aas:
        aas[_AA_1_UUID.encode(_ENCODING)] = _AA_1_VAL

    with closing(shelve.open(_DB_SRV_ACS, 'c')) as srv_acs:
        srv_acs[_SRV_1_UUID.encode(_ENCODING)] = _SRV_1_ACS
    with closing(shelve.open(_DB_SRV_GRPS, 'c')) as srv_grps:
        srv_grps[_SRV_1_UUID.encode(_ENCODING)] = _SRV_1_GRPS

    with closing(shelve.open(_DB_GRP_ACS, 'c')) as grp_acs:
        grp_acs[_GRP_1_UUID.encode(_ENCODING)] = _GRP_1_ACS
    with closing(shelve.open(_DB_GRP_OBJS, 'c')) as grp_objs:
        grp_objs[_GRP_1_UUID.encode(_ENCODING)] = _GRP_1_OBJS

    with closing(shelve.open(_DB_OBJ_ACS, 'c')) as obj_acs:
        obj_acs[_OBJ_1_UUID.encode(_ENCODING)] = _OBJ_1_ACS
    with closing(shelve.open(_DB_OBJ_VAL, 'c')) as obj_val:
        obj_val[_OBJ_1_UUID.encode(_ENCODING)] = _OBJ_1_VAL
    with closing(shelve.open(_DB_OBJ_VER, 'c')) as obj_ver:
        obj_val[_OBJ_1_UUID.encode(_ENCODING)] = _OBJ_1_VER
