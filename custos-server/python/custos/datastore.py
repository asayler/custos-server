#!/usr/bin/env python
# -*- coding: utf-8 -*-

import shelve

from abc import abstractmethod
from contextlib import closing

_ENCODING = 'utf-8'

DS_SRV_ACS = "ds_srv_acs"
DS_GRP_ACS = "ds_grp_acs"
DS_OBJ_ACS = "ds_obj_acs"

DS_SRV_GRP = "ds_srv_grp"
DS_GRP_OBJ = "ds_grp_obj"

DS_OBJ_VAL = "ds_obj_val"
DS_OBJ_VER_READ = "ds_obj_ver_read"
DS_OBJ_VER_UPDATE = "ds_obj_ver_update"

DS_AA = 'ds_aa'
DS_AA_ROW = { 'Class': None, 'Type': None, 'Value':None }

DS_ROW_MAP = { DS_AA: DS_AA_ROW }

class _DSbase(object):
    """
    Custos Generic database wrapper

    """

    def __init__(self, name):
        """Setup DS"""
        self._name = name

    @abstractmethod
    def __getitem__(self, key):
        """Get DS item"""

    @abstractmethod
    def __setitem__(self, key, val):
        """Set DS item"""

    @abstractmethod
    def __delitem__(self, key):
        """Delete DS item"""

    @abstractmethod
    def get_row(self, row_id):
        """Return DSrow item"""


class _DSshelve(_DSbase):
    """
    Custos shelve database driver

    """

    def __init__(self, name):
        """Setup DS"""
        super(_DSshelve, self).__init__(name)
        closing(shelve.open(self._name, 'c'))

    def __getitem__(self, key):
        """Get DS item"""
        with closing(shelve.open(self._name, 'r')) as s:
            return s[key]

    def __setitem__(self, key, val):
        """Set DS item"""
        with closing(shelve.open(self._name, 'w')) as s:
            s[key] = val
            return s[key]

    def __delitem__(self, key):
        """Delete DS item"""
        with closing(shelve.open(self._name, 'w')) as s:
            del(s[key])
            return None

    def get_row(self, row_id):
        """Return DSrow item"""
        row_proto = DS_ROW_MAP[self._name]
        return DSrow(self, row_id, row_proto)

class DSrow(object):
    """
    Custos database row

    """

    def __init__(self, ds, row_id, row_proto):
        """Setup DS"""
        self._ds = ds
        self.row_id = row_id
        self.prototype = row_proto
        if self.row_id not in self._ds:
            # Create prototype row
            self._ds[row_id] = self.prototype

    def __getitem__(self, key):
        """Get DS row item"""
        row = self.row()
        return row[key]

    def __setitem__(self, key, val):
        """Set DS row item"""
        row = self.row()
        row[key] = val
        self.set_row(row)
        return row[key]

    def __delitem__(self, key):
        """Delete DS row item"""
        pass

    def get_row(self):
        """Get DS row"""
        return self._ds[row_id]

    def set_row(self, row):
        """Set DS row"""
        self._ds[row_id] = row
        return self._ds[row_id]

class DS(_DSshelve):
    """
    Custos default database driver

    """

    pass
