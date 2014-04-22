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
        self.assertEqual(obj_a, obj_b, "Objects not equal")


class AATestCase(unittest.TestCase):

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_init(self):
        aa = ou.AA(TEST_UUID)
        self.assertEqual(str(aa.uuid), TEST_UUID_HEX, "str(aa.uuid) does not match UUID hex")
        self.assertEqual(repr(aa), TEST_UUID_HEX, "repr(aa) does not match UUID hex")

    def test_init_from_existing(self):
        aa = ou.AA.from_existing(TEST_UUID_HEX)
        self.assertEqual(str(aa.uuid), TEST_UUID_HEX, "str(aa.uuid) does not match UUID hex")
        self.assertEqual(repr(aa), TEST_UUID_HEX, "repr(aa) does not match UUID hex")

    def test_init_from_new(self):
        aa = ou.AA.from_new(TEST_AA_CLASS, TEST_AA_TYPE, TEST_AA_VALUE)
        self.assertEqual(str(aa.uuid), repr(aa), "str(obj.uuid) does not match repr(obj)")

    def test_equal(self):
        aa_a = ou.AA(TEST_UUID)
        aa_b = ou.AA.from_existing(TEST_UUID_HEX)
        self.assertEqual(aa_a, aa_b, "AAs not equal")


if __name__ == '__main__':
    unittest.main()
