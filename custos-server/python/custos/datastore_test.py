#!/usr/bin/env python
# -*- coding: utf-8 -*-

import unittest

import datastore

class DSTestCase(unittest.TestCase):

    def setUp(self):
        ds = datastore.DS(datastore.DS_TEST)
        if ds.exists():
            ds.destroy()

    def tearDown(self):
        ds = datastore.DS(datastore.DS_TEST)
        if ds.exists():
            ds.destroy()

    def test_init(self):
        ds = datastore.DS(datastore.DS_TEST)
        self.assertFalse(ds.exists(), "DS exists before create()")
        ds.create()
        self.assertTrue(ds.exists(), "DS does not exist after create()")
        ds.destroy()
        self.assertFalse(ds.exists(), "DS still exists after destroy()")

    def test_set(self):
        ds = datastore.DS(datastore.DS_TEST)
        ds.create()
        ds['key1'] = None
        self.assertTrue('key1' in ds, "Key1 does not exist")
        self.assertFalse('key2' in ds, "Key2 exists")
        ds.destroy()

    def test_set_len(self):
        ds = datastore.DS(datastore.DS_TEST)
        ds.create()
        ds['key1'] = None
        ds['key2'] = None
        ds['key3'] = None
        self.assertNotEqual(len(ds), 2, "DS has wrong length")
        self.assertEqual(len(ds), 3, "DS has wrong length")
        self.assertNotEqual(len(ds), 4, "DS has wrong length")
        ds.destroy()

    def test_set_get(self):
        ds = datastore.DS(datastore.DS_TEST)
        ds.create()
        ds['key1'] = None
        ds['key2'] = ""
        ds['key3'] = "This is key 3"
        ds['key4'] = 4
        ds['key5'] = []
        ds['key6'] = [1, 2, 3]
        ds['key7'] = {}
        ds['key8'] = {'a': 1, 'b':2, 'c':3}
        self.assertEqual(ds['key1'], None, "Key1 does not match")
        self.assertNotEqual(ds['key1'], 0, "Key1 does not match")
        self.assertEqual(ds['key2'], "", "Key2 does not match")
        self.assertNotEqual(ds['key2'], "   ", "Key2 does not match")
        self.assertEqual(ds['key3'], "This is key 3", "Key3 does not match")
        self.assertNotEqual(ds['key3'], "This is key 0", "Key3 does not match")
        self.assertEqual(ds['key4'], 4, "Key4 does not match")
        self.assertNotEqual(ds['key4'], 0, "Key4 does not match")
        self.assertEqual(ds['key5'], [], "Key5 does not match")
        self.assertNotEqual(ds['key5'], [0], "Key5 does not match")
        self.assertEqual(ds['key6'], [1, 2, 3], "Key6 does not match")
        self.assertNotEqual(ds['key6'], [3, 2, 1], "Key6 does not match")
        self.assertEqual(ds['key7'], {}, "Key7 does not match")
        self.assertNotEqual(ds['key7'], {'test': 0}, "Key7 does not match")
        self.assertEqual(ds['key8'], {'a': 1, 'b':2, 'c':3}, "Key8 does not match")
        self.assertNotEqual(ds['key8'], {'a': 3, 'b':2, 'c':1}, "Key8 does not match")
        ds.destroy()

    def test_set_del(self):
        ds = datastore.DS(datastore.DS_TEST)
        ds.create()
        ds['key1'] = None
        del(ds['key1'])
        self.assertFalse('key1' in ds)
        ds.destroy()

    def test_set_iter(self):
        ds = datastore.DS(datastore.DS_TEST)
        ds.create()
        ds['key1'] = None
        ds['key2'] = None
        ds['key3'] = None
        s = {'key1': None, 'key2': None, 'key3': None}
        for k in ds:
            self.assertTrue(k in s, "{} not in {}".format(k, s))
            del(s[k])
        self.assertEqual(len(s), 0, "{} still in s".format(s))
        ds.destroy()

    def test_set_iterkeys(self):
        ds = datastore.DS(datastore.DS_TEST)
        ds.create()
        ds['key1'] = None
        ds['key2'] = None
        ds['key3'] = None
        s = {'key1': None, 'key2': None, 'key3': None}
        for k in ds.iterkeys():
            self.assertTrue(k in s, "{} not in {}".format(k, s))
            del(s[k])
        self.assertEqual(len(s), 0, "{} still in s".format(s))
        ds.destroy()


class DSrowTestCase(unittest.TestCase):

    def setUp(self):
        ds = datastore.DS(datastore.DS_TEST)
        if ds.exists():
            ds.destroy()

    def tearDown(self):
        ds = datastore.DS(datastore.DS_TEST)
        if ds.exists():
            ds.destroy()

    def test_init(self):
        ds = datastore.DS(datastore.DS_TEST)
        ds.create()
        row = ds.row('row1')
        self.assertFalse(row.exists(), "Row exists before create()")
        row.create()
        self.assertTrue(row.exists(), "Row does not exist after create()")
        self.assertEqual(row.get_id(), 'row1', "Row ID does not match create ID")
        self.assertEqual(row.get_vals().keys(), datastore.DS_TEST_ROW.keys(),
                         "Row protoype does not match create prototype")
        self.assertEqual(row.get_vals().keys(), row.get_proto().keys(),
                         "Row protoype does not match stored prototype")
        row.destroy()
        self.assertFalse(row.exists(), "Row still exists after destroy()")
        ds.destroy()

    def test_set(self):
        ds = datastore.DS(datastore.DS_TEST)
        ds.create()
        row = ds.row('row1')
        row.create()
        key = row.get_proto().keys()[0]
        def setitem(k, v):
            row[k] = v
        setitem(key, 'fakeval')
        self.assertTrue(key in row, "{} does not exist".format(key))
        self.assertRaises(KeyError, setitem, 'fakekey', 'fakeval')
        self.assertFalse('fakekey' in row, "fakekey exists")
        row.destroy()
        ds.destroy()

    def test_set_len(self):
        ds = datastore.DS(datastore.DS_TEST)
        ds.create()
        row = ds.row('row1')
        row.create()
        self.assertEqual(len(row), len(datastore.DS_TEST_ROW), "Row has wrong length"),
        self.assertEqual(len(row), len(row.get_proto()), "Row has wrong length"),
        row.destroy()
        ds.destroy()

    def test_set_get(self):
        ds = datastore.DS(datastore.DS_TEST)
        ds.create()
        row = ds.row('row1')
        row.create()
        key = row.get_proto().keys()[0]
        row[key] = 'testval'
        def getitem(k):
            return row[k]
        self.assertEqual(row[key], 'testval', "{} val does not match".format(key))
        self.assertRaises(KeyError, getitem, 'fakekey')
        row.destroy()
        ds.destroy()

    def test_set_del(self):
        ds = datastore.DS(datastore.DS_TEST)
        ds.create()
        row = ds.row('row1')
        row.create()
        key = row.get_proto().keys()[0]
        self.assertEqual(row[key], row.get_proto()[key], "{} val does not match".format(key))
        row[key] = 'testval'
        self.assertEqual(row[key], 'testval', "{} val does not match".format(key))
        del(row[key])
        self.assertEqual(row[key], row.get_proto()[key], "{} val does not match".format(key))
        row.destroy()
        ds.destroy()

    def test_set_iter(self):
        ds = datastore.DS(datastore.DS_TEST)
        ds.create()
        row = ds.row('row1')
        row.create()
        s = row.get_proto()
        for k in row:
            self.assertTrue(k in s, "{} not in {}".format(k, s))
            del(s[k])
        self.assertEqual(len(s), 0, "{} still in s".format(s))
        row.destroy()
        ds.destroy()

    def test_set_iterkeys(self):
        ds = datastore.DS(datastore.DS_TEST)
        ds.create()
        row = ds.row('row1')
        row.create()
        s = row.get_proto()
        for k in row.iterkeys():
            self.assertTrue(k in s, "{} not in {}".format(k, s))
            del(s[k])
        self.assertEqual(len(s), 0, "{} still in s".format(s))
        row.destroy()
        ds.destroy()

if __name__ == '__main__':
    unittest.main()
