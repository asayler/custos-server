#!/usr/bin/env python
# -*- coding: utf-8 -*-

import shelve
import os

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


DS_TEST = 'ds_test'
DS_TEST_ROW = { 'col1': None, 'col2': None, 'col3':None }
DS_AA = 'ds_aa'
DS_AA_ROW = { 'Class': None, 'Type': None, 'Value':None }

DS_ROW_MAP = { DS_TEST: DS_TEST_ROW ,
               DS_AA: DS_AA_ROW }

class _DSbase(object):
    """
    Custos Generic database wrapper

    """

    def __init__(self, name):
        """Setup DS"""
        self._name = name

    @abstractmethod
    def __len__(self):
        """Number of DS rows"""

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
    def __contains__(self, key):
        """Test for DS item existance"""

    @abstractmethod
    def create(self, overwrite=False):
        """Create DS"""

    @abstractmethod
    def destroy(self):
        """Destroy DS"""

    @abstractmethod
    def exits(self):
        """Test for DS Existance"""

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

    def __len__(self):
        """Number of DS rows"""
        with closing(shelve.open(self._name, 'r')) as s:
            return len(s)

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

    def __contains__(self, key):
        """Test for DS item existance"""
        with closing(shelve.open(self._name, 'r')) as s:
            return key in s

    def create(self, overwrite=False):
        """Create DS"""
        if overwrite:
            closing(shelve.open(self._name, 'n'))
        else:
            closing(shelve.open(self._name, 'c'))

    def destroy(self):
        """Destory DS"""
        os.remove(self._name)

    def exists(self):
        """Test for DS Existance"""
        return os.path.isfile(self._name)

    def row(self, row_id):
        """Return DSrow item"""
        row_proto = DS_ROW_MAP[self._name]
        return DSrow(self, row_id, row_proto)


class DS(_DSshelve):
    """
    Custos default database driver

    """

    pass

class DSrow(object):
    """
    Custos database row

    """

    def __init__(self, ds, row_id, row_proto):
        """Setup DS"""
        self._ds = ds
        self.row_id = row_id
        self.prototype = row_proto

    def __getitem__(self, key):
        """Get DS row item"""
        row = self.get_row()
        return row[key]

    def __setitem__(self, key, val):
        """Set DS row item"""
        row = self.get_row()
        row[key] = val
        self.set_row(row)
        return row[key]

    def __delitem__(self, key):
        """Delete DS row item"""
        pass

    def create(self, overwrite=False):
        """Create DS row"""
        if overwrite:
            self._ds[row_id] = self.prototype
        else:
            if self.row_id not in self._ds:
                self._ds[self.row_id] = self.prototype
            else:
                pass

    def destroy(self):
        """Destory DS row"""
        del(self._ds[self.row_id])

    def exists(self):
        """Test for DS Existance"""
        return self.row_id in self._ds

    def get_row(self):
        """Get DS row"""
        return self._ds[self.row_id]

    def set_row(self, row):
        """Set DS row"""
        self._ds[self.row_id] = row
        return self._ds[self.row_id]
