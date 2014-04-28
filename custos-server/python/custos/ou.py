#!/usr/bin/env python
# -*- coding: utf-8 -*-

import uuid

from abc import abstractmethod
from contextlib import closing

import datastore

_ENCODING = 'utf-8'

# Exceptions

class OUError(Exception):
    """Base class for OU exceptions"""

    pass

class UUIDObjectError(OUError):
    """Base class for OU UUID Object excpetions"""

    pass

class UUIDObjectDoesNotExistError(UUIDObjectError):
    """UUID Object does not exist"""

    def __init__(self, uuid_object):
        self.msg = "{} does not exist".format(uuid_object)

# Objects

class UUIDObject(object):
    """
    Custos UUID Object Base Class

    """

    def __init__(self, obj_uuid):
        """Base Constructor"""
        self.uuid = obj_uuid

    @classmethod
    def from_new(cls):
        """New Constructor"""
        obj_uuid = uuid.uuid4()
        return cls(obj_uuid)

    @classmethod
    def from_existing(cls, uuid_hex):
        """Exisiting Constructor"""
        obj_uuid = uuid.UUID(uuid_hex)
        return cls(obj_uuid)

    def __unicode__(self):
        u = u"{:s}_{:012x}".format(type(self).__name__, self.uuid.node)
        return u

    def __str__(self):
        return unicode(self).encode(_ENCODING)

    def __repr__(self):
        u = u"{:s}".format(self.uuid)
        return u.encode(_ENCODING)

    def __hash__(self):
        return hash(repr(self))

    def __eq__(self, other):
        return (repr(self) == repr(other))


class AA(UUIDObject):
    """
    Access Attribute

    """

    def __init__(self, aa_uuid):
        """Base Constructor"""
        super(AA, self).__init__(aa_uuid)
        self._ds = datastore.DS(datastore.DS_AA)
        self._ds_row = self._ds.row(repr(self))

    @classmethod
    def from_new(cls, aa_class, aa_type, aa_value):
        """New Constructor"""
        aa = super(AA, cls).from_new()
        vals = { 'Class': aa_class, 'Type': aa_type, 'Value': aa_value }
        aa._ds_row.create()
        aa._ds_row.set_vals(vals)
        return aa

    @classmethod
    def from_existing(cls, uuid_hex):
        """Exisiting Constructor"""
        aa = super(AA, cls).from_existing(uuid_hex)
        if not aa._ds_row.exists():
            raise UUIDObjectDoesNotExistError(aa)
        return aa

    def __getitem__(self, key):
        return self._ds_row[key]

    def __setitem__(self, key, value):
        self._ds_row[key] = value


# class custos_acc(object):
#     """
#     List of Access Attributes

#     """

#     def __init__(self, acc_uuid=None):
#         if not acc_uuid:
#             acc_uuid = unicode(uuid.uuid4())
#         self.uuid = acc_uuid

#     def __repr__(self):
#         return u"acc_" + self.uuid

# class custos_acg(object):
#     """
#     Group of Access Control Chains

#     """

#     def __init__(self, acg_uuid=None):
#         if not acg_uuid:
#             acg_uuid = unicode(uuid.uuid4())
#         self.uuid = acg_uuid

#     def __repr__(self):
#         return u"acg_" + self.uuid

# class custos_acs(object):
#     """
#     Specification of permissions to ACGs

#     """

#     def __init__(self, acs_uuid=None):
#         if not acs_uuid:
#             acs_uuid = unicode(uuid.uuid4())
#         self.uuid = acs_uuid

#     def __repr__(self):
#         return u"acs_" + self.uuid
