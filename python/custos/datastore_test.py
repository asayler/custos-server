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

    def test_set_get(self):
        ds = datastore.DS(DS_TEST)
        ds.create()
        self.assertTrue(ds.exists(), "DS does not exist after create()")
        ds['key1'] = "This is key 1"
        ds['key2'] = "This is key 2"
        ds['key3'] = "This is key 3"
        self.assertEqual(ds['key1'], "This is key 1", "Key1 does not match")
        self.assertEqual(ds['key2'], "This is key 2", "Key2 does not match")
        self.assertEqual(ds['key3'], "This is key 3", "Key3 does not match")
        ds.destroy()
        self.assertFalse(ds.exists(), "DS still exists after destroy()")

    def test_set_get_del(self):
        ds = datastore.DS(DS_TEST)
        ds.create()
        f = lambda k: ds[k]
        self.assertTrue(ds.exists(), "DS does not exist after create()")
        ds['key1'] = "This is key 1"
        ds['key2'] = "This is key 2"
        self.assertEqual(ds['key1'], "This is key 1", "Key1 does not match")
        self.assertEqual(ds['key2'], "This is key 2", "Key2 does not match")
        del(ds['key1'])
        self.assertRaises(KeyError, f, 'key1')
        self.assertEqual(ds['key2'], "This is key 2", "Key2 does not match")
        del(ds['key2'])
        self.assertRaises(KeyError, f, 'key1')
        self.assertRaises(KeyError, f, 'key2')
        ds.destroy()
        self.assertFalse(ds.exists(), "DS still exists after destroy()")


if __name__ == '__main__':
    unittest.main()
