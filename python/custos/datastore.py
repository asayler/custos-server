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

DS_AA = "ds_aa"


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


class _DSshelve(_DSbase):
    """
    Custos shelve database driver

    """

    def __init__(self, name):
        super(_DSshelve, self).__init__(name)
        closing(shelve.open(self._name, 'c'))

    def __getitem__(self, key):
        with closing(shelve.open(self._name, 'r')) as s:
            return s[key]

    def __setitem__(self, key, val):
        with closing(shelve.open(self._name, 'w')) as s:
            s[key] = val
            return s[key]

    def __delitem__(self, key):
        with closing(shelve.open(self._name, 'w')) as s:
            del(s[key])
            return None


class DS(_DSshelve):
    """
    Custos default database driver

    """

    pass
