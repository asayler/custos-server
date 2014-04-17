#!/usr/bin/env python
# -*- coding: utf-8 -*-

import unittest

import datastore

DS_TEST = 'unittest'

class DSTestCase(unittest.TestCase):

    def setUp(self):
        ds = datastore.DS(DS_TEST)
        if ds.exists():
            ds.destroy()

    def tearDown(self):
        ds = datastore.DS(DS_TEST)
        if ds.exists():
            ds.destroy()

    def test_init(self):
        ds = datastore.DS(DS_TEST)
        ds.create()
        self.assertTrue(ds.exists(), "DS does not exist after create()")
        ds.destroy()
        self.assertFalse(ds.exists(), "DS still exists after destroy()")

    # def test_set(self):
    #     ds = DS(DS_TEST)
    #     ds.create()
    #     ds['key1'] = "This is key 1"
    #     ds['key2'] = "This is key 2"
    #     ds['key3'] = "This is key 3"

if __name__ == '__main__':
    unittest.main()
