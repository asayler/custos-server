#!/usr/bin/env python
# -*- coding: utf-8 -*-

import copy
import os
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


DS_TEST = 'ds_test'
DS_TEST_ROW = { 'col1': None, 'col2': None, 'col3':None }
DS_AA = 'ds_aa'
DS_AA_ROW = { 'Class': None, 'Type': None, 'Value':None }

DS_ROW_MAP = { DS_TEST: DS_TEST_ROW ,
               DS_AA: DS_AA_ROW }

# Exceptions

class DSError(Exception):
    """Base class for DS excpetions"""

    pass

class DSrowError(DSError):
    """Base class for DS row excpetions"""

    pass

class DSrowFormatError(DSrowError):
    """DS row exceptions for prototype format mismatch"""

    def __init__(self, msg):
        self.msg = msg

# DS Classes

class _DSbase(object):
    """
    Custos Generic database wrapper

    """

    def __init__(self, name):
        """Setup DS"""
        self._name = copy.deepcopy(name)

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
    def __iter__(self):
        """Get DS key iterator"""

    @abstractmethod
    def iterkeys(self):
        """Get DS key iterator"""

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
    def get_name(self):
        """Return DS name"""

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

    def __len__(self):
        """Number of DS rows"""
        with closing(shelve.open(self._name, 'r')) as s:
            return len(s)

    def __getitem__(self, key):
        """Get DS item"""
        with closing(shelve.open(self._name, 'r')) as s:
            return copy.deepcopy(s[key])

    def __setitem__(self, key, val):
        """Set DS item"""
        with closing(shelve.open(self._name, 'w')) as s:
            s[key] = copy.deepcopy(val)
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

    def __iter__(self):
        """Get DS key iterator"""
        with closing(shelve.open(self._name, 'r')) as s:
            for key in s.keys():
                yield key

    def iterkeys(self):
        """Get DS key iterator"""
        return self.__iter__()

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
        """Setup DS row"""
        self._ds = ds
        self._id = copy.deepcopy(row_id)
        self._proto = copy.deepcopy(row_proto)

    def __len__(self):
        """Number of DS row cols"""
        vals = self.get_vals()
        return len(vals)

    def __getitem__(self, key):
        """Get DS row item"""
        vals = self.get_vals()
        return vals[key]

    def __setitem__(self, key, val):
        """Set DS row item"""
        vals = self.get_vals()
        if key in vals:
            vals[key] = copy.deepcopy(val)
            self.set_vals(vals)
            return vals[key]
        else:
            raise KeyError ("{} not in {}".format(key, self._proto.keys()))

    def __delitem__(self, key):
        """Delete DS row item"""
        vals = self.get_vals()
        if key in vals:
            vals[key] = copy.deepcopy(self._proto[key])
            self.set_vals(vals)
            return vals[key]
        else:
            raise KeyError ("{} not in {}".format(key, self._proto.keys()))

    def __contains__(self, key):
        """Test for DS row item existance"""
        vals = self.get_vals()
        return key in vals

    def __iter__(self):
        """Get DS row key iterator"""
        vals = self.get_vals()
        for key in vals:
            yield key

    def iterkeys(self):
        """Get DS row key iterator"""
        return self.__iter__()

    def create(self, overwrite=False):
        """Create DS row"""
        if overwrite:
            self._ds[self._id] = copy.deepcopy(self._proto)
        else:
            if self._id not in self._ds:
                self._ds[self._id] = copy.deepcopy(self._proto)
            else:
                pass

    def destroy(self):
        """Destory DS row"""
        del(self._ds[self._id])

    def exists(self):
        """Test for DS Existance"""
        return self._id in self._ds

    def get_id(self):
        """Get DS row ID"""
        return copy.deepcopy(self._id)

    def get_proto(self):
        """Get DS row prototype"""
        return copy.deepcopy(self._proto)

    def get_vals(self):
        """Get DS row"""
        return copy.deepcopy(self._ds[self._id])

    def set_vals(self, vals):
        """Set DS row"""
        if vals.keys() == self._proto.keys():
            self._ds[self._id] = copy.deepcopy(vals)
            return self._ds[self._id]
        else:
            raise DSrowFormatError("{} does not match {}".format(vals.keys(), self._proto.keys()))
