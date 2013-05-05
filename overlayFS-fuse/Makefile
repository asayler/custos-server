# File: Makefile
# By: Andy Sayler <www.andysayler.com>
# Adopted from work by: Chris Wailes <chris.wailes@gmail.com>
# Project: CSCI 3753 Programming Assignment 5
# Creation Date: 2010/04/06
# Modififed Date: 2012/04/12
# Description:
#	This is the Makefile for PA5.


CC           = gcc

CFLAGSFUSE   = `pkg-config fuse --cflags`
LLIBSFUSE    = `pkg-config fuse --libs`
LLIBSULOCK   = -lulockmgr
LLIBSOPENSSL = -lcrypto
LLIBSUUID    = -luuid
LLIBSCURL    = -lcurl

CFLAGS = -c -g -Wall -Wextra
LFLAGS = -g -Wall -Wextra

ENCFS            = fuseenc fuseenc_fh
MIRFS            = fusemir_fh
FUSE_EXAMPLES    = fusehello fusexmp fusexmp_fh
XATTR_EXAMPLES   = xattr-util
OPENSSL_EXAMPLES = aes-crypt-util 
CUSTOS_UTIL      = custos_client_util
CURL_TEST        = curl-test

.PHONY: all encfs mirfs fuse-examples xattr-examples openssl-examples custos_util curl_test clean

all: encfs mirfs fuse-examples xattr-examples openssl-examples custos_util curl_test

encfs: $(ENCFS)
mirfs: $(MIRFS)
fuse-examples: $(FUSE_EXAMPLES)
xattr-examples: $(XATTR_EXAMPLES)
openssl-examples: $(OPENSSL_EXAMPLES)
custos_util: $(CUSTOS_UTIL)
curl_test: $(CURL_TEST)

fusehello: fusehello.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSFUSE)

fusexmp: fusexmp.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSFUSE)

fusexmp_fh: fusexmp_fh.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSFUSE) $(LLIBSULOCK)

fuseenc: fuseenc.o aes-crypt.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSFUSE) $(LLIBSOPENSSL)

fuseenc_fh: fuseenc_fh.o aes-crypt.o custos_client.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSFUSE) $(LLIBSULOCK) $(LLIBSOPENSSL) $(LLIBSUUID)

fusemir_fh: fusemir_fh.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSFUSE) $(LLIBSULOCK)

xattr-util: xattr-util.o
	$(CC) $(LFLAGS) $^ -o $@

aes-crypt-util: aes-crypt-util.o aes-crypt.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSOPENSSL)

custos_client_util: custos_client_util.o custos_client.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSUUID)

curl-test: curl-test.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSCURL)

fusehello.o: fusehello.c
	$(CC) $(CFLAGS) $(CFLAGSFUSE) $<

fusexmp.o: fusexmp.c
	$(CC) $(CFLAGS) $(CFLAGSFUSE) $<

fusexmp_fh.o: fusexmp_fh.c
	$(CC) $(CFLAGS) $(CFLAGSFUSE) $<

fuseenc.o: fuseenc.c
	$(CC) $(CFLAGS) $(CFLAGSFUSE) $<

fuseenc_fh.o: fuseenc_fh.c aes-crypt.h
	$(CC) $(CFLAGS) $(CFLAGSFUSE) $<

fusemir_fh.o: fusemir_fh.c
	$(CC) $(CFLAGS) $(CFLAGSFUSE) $<

xattr-util.o: xattr-util.c
	$(CC) $(CFLAGS) $<

aes-crypt-util.o: aes-crypt-util.c aes-crypt.h
	$(CC) $(CFLAGS) $<

aes-crypt.o: aes-crypt.c aes-crypt.h
	$(CC) $(CFLAGS) $<

custos_client_util.o: custos_client_util.c custos_client.h
	$(CC) $(CFLAGS) $<

custos_client.o: custos_client.c custos_client.h
	$(CC) $(CFLAGS) $<

curl-test.o: curl-test.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f $(ENCFS)
	rm -f $(MIRFS)
	rm -f $(FUSE_EXAMPLES)
	rm -f $(XATTR_EXAMPLES)
	rm -f $(OPENSSL_EXAMPLES)
	rm -f $(CUSTOS_UTIL)
	rm -f $(CURL_TEST)
	rm -f *.o
	rm -f *~
	rm -f handout/*~
	rm -f handout/*.log
	rm -f handout/*.aux
	rm -f handout/*.out



