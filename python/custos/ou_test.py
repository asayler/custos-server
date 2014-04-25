#!/usr/bin/env python
# -*- coding: utf-8 -*-

import unittest
import uuid

import ou

TEST_UUID_HEX = '4e4b1408-43d2-43f9-bc0a-59b7cb88f781'
TEST_UUID = uuid.UUID(TEST_UUID_HEX)

TEST_AA_CLASS = 'TestClass'
TEST_AA_TYPE = 'TestType'
TEST_AA_VALUE = 'TestValue'

class UUIDObjectTestCase(unittest.TestCase):

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_init(self):
        obj = ou.UUIDObject(TEST_UUID)
        self.assertEqual(str(obj.uuid), TEST_UUID_HEX, "str(obj.uuid) does not match UUID hex")
        self.assertEqual(repr(obj), TEST_UUID_HEX, "repr(obj) does not match UUID hex")

    def test_init_from_existing(self):
        obj = ou.UUIDObject.from_existing(TEST_UUID_HEX)
        self.assertEqual(str(obj.uuid), TEST_UUID_HEX, "str(obj.uuid) does not match UUID hex")
        self.assertEqual(repr(obj), TEST_UUID_HEX, "repr(obj) does not match UUID hex")

    def test_init_from_new(self):
        obj = ou.UUIDObject.from_new()
        self.assertEqual(str(obj.uuid), repr(obj), "str(obj.uuid) does not match repr(obj)")

    def test_equal(self):
        obj_a = ou.UUIDObject(TEST_UUID)
        obj_b = ou.UUIDObject.from_existing(TEST_UUID_HEX)
        obj_c = ou.UUIDObject.from_new()
        obj_d = ou.UUIDObject.from_new()
        self.assertEqual(obj_a, obj_b, "Objects not equal")
        self.assertNotEqual(obj_c, obj_d, "Objects equal")


class AATestCase(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        ou.datastore.DS_AA_OLD = ou.datastore.DS_AA
        ou.datastore.DS_AA += '_test'
        ou.datastore.DS_ROW_MAP_OLD = ou.datastore.DS_ROW_MAP
        ou.datastore.DS_ROW_MAP[ou.datastore.DS_AA] = ou.datastore.DS_AA_ROW

    @classmethod
    def tearDownClass(cls):
        ou.datastore.DS_ROW_MAP = ou.datastore.DS_ROW_MAP_OLD
        del(ou.datastore.DS_ROW_MAP_OLD)
        ou.datastore.DS_AA = ou.datastore.DS_AA_OLD
        del(ou.datastore.DS_AA_OLD)

    def setUp(self):
        ds = ou.datastore.DS(ou.datastore.DS_AA)
        ds.create(overwrite=True)

    def tearDown(self):
        ds = ou.datastore.DS(ou.datastore.DS_AA)
        ds.destroy()

    def test_init(self):
        aa = ou.AA(TEST_UUID)
        self.assertEqual(str(aa.uuid), TEST_UUID_HEX, "str(aa.uuid) does not match UUID hex")
        self.assertEqual(repr(aa), TEST_UUID_HEX, "repr(aa) does not match UUID hex")

    def test_init_from_new(self):
        aa = ou.AA.from_new(TEST_AA_CLASS, TEST_AA_TYPE, TEST_AA_VALUE)
        self.assertEqual(str(aa.uuid), repr(aa), "str(obj.uuid) does not match repr(obj)")

    def test_init_from_existing(self):
        aa_a = ou.AA.from_new(TEST_AA_CLASS, TEST_AA_TYPE, TEST_AA_VALUE)
        aa_b = ou.AA.from_existing(repr(aa_a))
        self.assertEqual(str(aa_b.uuid), repr(aa_b), "str(obj.uuid) does not match repr(obj)")

    def test_equal(self):
        aa_a = ou.AA.from_new(TEST_AA_CLASS, TEST_AA_TYPE, TEST_AA_VALUE)
        aa_b = ou.AA.from_existing(repr(aa_a))
        aa_c = ou.AA.from_new(TEST_AA_CLASS, TEST_AA_TYPE, TEST_AA_VALUE)
        self.assertEqual(aa_a, aa_b, "AAs not equal")
        self.assertNotEqual(aa_b, aa_c, "AAs equal")

    def test_get_item(self):
        aa = ou.AA.from_new(TEST_AA_CLASS, TEST_AA_TYPE, TEST_AA_VALUE)
        def get_item(k):
            return aa[k]
        self.assertEqual(aa['Class'], TEST_AA_CLASS, ("AA Class Mismatch: {} != {}"
                                                      .format(aa['Class'], TEST_AA_CLASS)))
        self.assertEqual(aa['Type'], TEST_AA_TYPE, ("AA Type Mismatch: {} != {}"
                                                    .format(aa['Type'], TEST_AA_TYPE)))
        self.assertEqual(aa['Value'], TEST_AA_VALUE, ("AA Value Mismatch: {} != {}"
                                                      .format(aa['Value'], TEST_AA_VALUE)))
        self.assertRaises(KeyError, get_item, 'FakeKey')

    def test_set_item(self):
        aa = ou.AA.from_new("", "", "")
        def set_item(k, v):
            aa[k] = v
        def get_item(k):
            return aa[k]
        aa['Class'] = TEST_AA_CLASS
        aa['Type'] = TEST_AA_TYPE
        aa['Value'] = TEST_AA_VALUE
        self.assertRaises(KeyError, set_item, 'FakeKey', 'FakeValue')
        self.assertEqual(aa['Class'], TEST_AA_CLASS, ("AA Class Mismatch: {} != {}"
                                                      .format(aa['Class'], TEST_AA_CLASS)))
        self.assertEqual(aa['Type'], TEST_AA_TYPE, ("AA Type Mismatch: {} != {}"
                                                    .format(aa['Type'], TEST_AA_TYPE)))
        self.assertEqual(aa['Value'], TEST_AA_VALUE, ("AA Value Mismatch: {} != {}"
                                                      .format(aa['Value'], TEST_AA_VALUE)))
        self.assertRaises(KeyError, get_item, 'FakeKey')


if __name__ == '__main__':
    unittest.main()
