#!/usr/bin/env python
# -*- coding: utf-8 -*-

import shelve
import uuid as py_uuid

from contextlib import closing

_ENCODING = 'utf-8'
_VER_SEPERATOR = '+'

_INIT_SRV_GRPS = []
_INIT_GRP_OBJS = []
_INIT_OBJ_VER = 1

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

_OBJ_1_ACS = { u"obj_delete": _ACL_1,
               u"obj_read": _ACL_1,
               u"obj_update": _ACL_1,
               u"obj_audit": _ACL_1,
               u"obj_clean": _ACL_1,
               u"obj_acs_get": _ACL_1,
               u"obj_acs_set": _ACL_1 }
_OBJ_1_VAL  = u"VGhpcyBpcyBhIHNlY3JldCBrZXkhAA=="

_GRP_1_ACS = { u"grp_obj_create": _ACL_1,
               u"grp_obj_list": _ACL_1,
               u"grp_obj_ovr": _ACL_1,
               u"grp_delete": _ACL_1,
               u"grp_audit": _ACL_1,
               u"grp_clean": _ACL_1,
               u"grp_acs_get": _ACL_1,
               u"grp_acs_set": _ACL_1 }

_SRV_1_UUID = u"30976aa2-fcf9-463e-a6ed-ba7e3ef6ebd4"
_SRV_1_ACS = { u"srv_grp_create": _ACL_1,
               u"srv_grp_list": _ACL_1,
               u"srv_grp_ovr": _ACL_1,
               u"srv_audit": _ACL_1,
               u"srv_clean": _ACL_1,
               u"srv_acs_get": _ACL_1,
               u"srv_acs_set": _ACL_1 }

# Get OUs

def get_attr_val(aa_uuid, ver=None):

    with closing(shelve.open(_DB_AAS, 'r')) as aas:
        if aa_uuid.encode(_ENCODING) in aas:
            return aas[aa_uuid.encode(_ENCODING)]
        else:
            return None


# Objects Methods

class custos_srv:

    def __init__(self, srv_uuid=None):
        if not srv_uuid:
            srv_uuid = _SRV_1_UUID
        self.uuid = srv_uuid

    def get_ACS(self):

        srv_uuid_str = self.uuid.encode(_ENCODING)

        with closing(shelve.open(_DB_SRV_ACS, 'r')) as db_srv_acs:
            if srv_uuid_str in db_srv_acs:
                return db_srv_acs[srv_uuid_str]
            else:
                return None

    def list_grps(self):

        srv_uuid_str = self.uuid.encode(_ENCODING)

        with closing(shelve.open(_DB_SRV_GRPS, 'r')) as db_srv_grps:
            if srv_uuid_str in db_srv_grps:
                return db_srv_grps[srv_uuid_str]
            else:
                return None

    def create_grp(self, grp_acs):

        srv_uuid_str = self.uuid.encode(_ENCODING)

        grp_uuid = unicode(py_uuid.uuid4())
        grp_uuid_str = grp_uuid.encode(_ENCODING)

        with closing(shelve.open(_DB_GRP_ACS, 'w')) as db_grp_acs:
            db_grp_acs[grp_uuid_str] = grp_acs
        with closing(shelve.open(_DB_GRP_OBJS, 'w')) as db_grp_objs:
            db_grp_objs[grp_uuid_str] = _INIT_GRP_OBJS

        with closing(shelve.open(_DB_SRV_GRPS, 'w')) as db_srv_grps:
            tmp = db_srv_grps[srv_uuid_str]
            tmp.append(grp_uuid)
            db_srv_grps[srv_uuid_str] = tmp

        return custos_grp(grp_uuid)


class custos_grp:

    def __init__(self, grp_uuid):
        self.uuid = grp_uuid

    def get_ACS(self):

        grp_uuid_str = self.uuid.encode(_ENCODING)

        with closing(shelve.open(_DB_GRP_ACS, 'r')) as db_grp_acs:
            if grp_uuid_str in db_grp_acs:
                return db_grp_acs[grp_uuid_str]
            else:
                return None

    def list_objs(self):

        grp_uuid_str = self.uuid.encode(_ENCODING)

        with closing(shelve.open(_DB_GRP_OBJS, 'r')) as db_grp_objs:
            if grp_uuid_str in db_grp_objs:
                return db_grp_objs[grp_uuid_str]
            else:
                return None

    def create_obj(self, obj_acs, obj_val):

        grp_uuid_str = self.uuid.encode(_ENCODING)

        obj_uuid = unicode(py_uuid.uuid4())
        obj_ver = _INIT_OBJ_VER
        obj_uuid_str = obj_uuid.encode(_ENCODING)
        obj_uuid_ver_str = _build_uuid_ver(obj_uuid, obj_ver).encode(_ENCODING)

        with closing(shelve.open(_DB_OBJ_VER, 'w')) as db_obj_ver:
            db_obj_ver[obj_uuid_str] = obj_ver
        with closing(shelve.open(_DB_OBJ_ACS, 'w')) as db_obj_acs:
            db_obj_acs[obj_uuid_ver_str] = obj_acs
        with closing(shelve.open(_DB_OBJ_VAL, 'w')) as db_obj_val:
            db_obj_val[obj_uuid_ver_str] = obj_val

        with closing(shelve.open(_DB_GRP_OBJS, 'w')) as db_grp_objs:
            tmp = db_grp_objs[grp_uuid_str]
            tmp.append(obj_uuid)
            db_grp_objs[grp_uuid_str] = tmp

        return custos_obj(obj_uuid)


class custos_obj:

    def __init__(self, obj_uuid):
        self.uuid = obj_uuid

    def get_ver(self):

        uuid_str = self.uuid.encode(_ENCODING)

        with closing(shelve.open(_DB_OBJ_VER, 'r')) as db_obj_ver:
            if uuid_str in db_obj_ver:
                return db_obj_ver[uuid_str]
            else:
                return None

    def get_ACS(self, obj_ver=None):

        if not obj_ver:
            obj_ver = self.get_ver()
            if not obj_ver:
                return None

        uuid_ver_str = _build_uuid_ver(self.uuid, obj_ver).encode(_ENCODING)

        with closing(shelve.open(_DB_OBJ_ACS, 'r')) as db_obj_acs:
            if uuid_ver_str in db_obj_acs:
                return db_obj_acs[uuid_ver_str], obj_ver
            else:
                return None

    def get_val(self, obj_ver=None):

        if not obj_ver:
            obj_ver = self.get_ver()
            if not obj_ver:
                return None

        uuid_ver_str = _build_uuid_ver(self.uuid, obj_ver).encode(_ENCODING)

        with closing(shelve.open(_DB_OBJ_VAL, 'r')) as db_obj_val:
            if uuid_ver_str in db_obj_val:
                return db_obj_val[uuid_ver_str], obj_ver
            else:
                return None


# Utilty Functions

def _build_uuid_ver(uuid, ver):

    return uuid + _VER_SEPERATOR + unicode(ver)

# Main: Setup DBs

if __name__ == "__main__":

    with closing(shelve.open(_DB_AAS, 'n')) as db_aas:
        db_aas[_AA_1_UUID.encode(_ENCODING)] = _AA_1_VAL

    with closing(shelve.open(_DB_SRV_ACS, 'n')) as db_srv_acs:
        db_srv_acs[_SRV_1_UUID.encode(_ENCODING)] = _SRV_1_ACS
    with closing(shelve.open(_DB_SRV_GRPS, 'n')) as db_srv_grps:
        db_srv_grps[_SRV_1_UUID.encode(_ENCODING)] = _INIT_SRV_GRPS

    with closing(shelve.open(_DB_GRP_ACS, 'n')) as db_grp_acs:
        pass
    with closing(shelve.open(_DB_GRP_OBJS, 'n')) as db_grp_objs:
        pass

    with closing(shelve.open(_DB_OBJ_ACS, 'n')) as db_obj_acs:
        pass
    with closing(shelve.open(_DB_OBJ_VAL, 'n')) as db_obj_val:
        pass
    with closing(shelve.open(_DB_OBJ_VER, 'n')) as db_obj_ver:
        pass

    srv_1 = custos_srv()
    grp_1 = srv_1.create_grp(_GRP_1_ACS)
    obj_1 = grp_1.create_obj(_OBJ_1_ACS, _OBJ_1_VAL)

    print("groups: {:s}".format(srv_1.list_grps()))
    print("objects: {:s}".format(grp_1.list_objs()))
