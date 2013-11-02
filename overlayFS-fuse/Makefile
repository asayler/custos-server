# File: Makefile
# By: Andy Sayler <www.andysayler.com>
# Build Custos Filesystem Demo

AR            = ar
CC            = gcc

CFLAGS  = -c -g -Wall -Wextra -Werror
LFLAGS  = -g -Wall -Wextra -Werror
ARFLAGS = rcsv

# Executables
ENCFS              = fuseenc fuseenc_fh
MIRFS              = fusemir_fh
FUSE_EXAMPLES      = fusehello fusexmp fusexmp_fh
XATTR_EXAMPLES     = xattr-util
OPENSSL_EXAMPLES   = aes-crypt-util
CURL_EXAMPLES      = curl_example
CUSTOS_TESTS       = custos_client_test custos_http_test custos_json_test custos_decode_test

CUSTOS_LIB         = libcustos.a
CUSTOS_LIB_OBJECTS = custos_client.o custos_client_print.o \
	             custos_http.o custos_json.o \
	             ../libb64-1.2.1/src/cdecode.o ../libb64-1.2.1/src/cencode.o

CFLAGSFUSE    = `pkg-config fuse --cflags`
LLIBSFUSE     = `pkg-config fuse --libs`
CFLAGSULOCK   =
LLIBSULOCK    = -lulockmgr
CFLAGSOPENSSL = `pkg-config openssl --cflags`
LLIBSOPENSSL  = `pkg-config openssl --libs`
CFLAGSUUID    = `pkg-config uuid --cflags`
LLIBSUUID     = `pkg-config uuid --libs`
CFLAGSCURL    = `pkg-config libcurl --cflags`
LLIBSCURL     = `pkg-config libcurl --libs`
CFLAGSJSON    = `pkg-config json --cflags`
LLIBSJSON     = `pkg-config json --libs`
CFLAGSMHASH   =
LLIBSMHASH    = -lmhash

.PHONY: all tests encfs mirfs \
	fuse-examples xattr-examples openssl-examples \
	custos_tests curl_examples \
	clean

all: encfs mirfs \
	fuse-examples xattr-examples openssl-examples \
	tests examples

tests: custos_tests

examples: curl_examples

encfs: $(ENCFS)
mirfs: $(MIRFS)
fuse-examples: $(FUSE_EXAMPLES)
xattr-examples: $(XATTR_EXAMPLES)
openssl-examples: $(OPENSSL_EXAMPLES)
curl_examples: $(CURL_EXAMPLES)
custos_tests: $(CUSTOS_TESTS)

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

custos_client_test: custos_client_test.o $(CUSTOS_LIB)
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSCURL) $(LLIBSJSON) $(LLIBSUUID) $(LLIBSMHASH)

custos_decode_test: custos_decode_test.o $(CUSTOS_LIB)
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSJSON) $(LLIBSCURL) $(LLIBSMHASH)

custos_json_test: custos_json_test.o $(CUSTOS_LIB)
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSJSON)

custos_http_test: custos_http_test.o $(CUSTOS_LIB)
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSCURL) $(LLIBSMHASH)

$(CUSTOS_LIB): $(CUSTOS_LIB_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

curl_example: curl_example.o
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
	$(CC) $(CFLAGS) $(CFLAGSFUSE) $(CFLAGSUUID) $<

fusemir_fh.o: fusemir_fh.c
	$(CC) $(CFLAGS) $(CFLAGSFUSE) $<

xattr-util.o: xattr-util.c
	$(CC) $(CFLAGS) $<

aes-crypt-util.o: aes-crypt-util.c aes-crypt.h
	$(CC) $(CFLAGS) $<

aes-crypt.o: aes-crypt.c aes-crypt.h
	$(CC) $(CFLAGS) $(CFLAGSOPENSSL) $<

custos_client_test.o: custos_client_test.c custos_client.h custos_client_print.h
	$(CC) $(CFLAGS) $(CFLAGSUUID) $<

custos_client.o: custos_client.c custos_client.h
	$(CC) $(CFLAGS) $(CFLAGSUUID) $<

custos_client_print.o: custos_client_print.c custos_client_print.h custos_client.h
	$(CC) $(CFLAGS) $(CFLAGSUUID) $<

custos_http.o: custos_http.c custos_http.h
	$(CC) $(CFLAGS) $(CFLAGSCURL) $<

custos_json.o: custos_json.c custos_json.h
	$(CC) $(CFLAGS) $(CFLAGSJSON) $<

custos_http_test.o: custos_http_test.c custos_http.h
	$(CC) $(CFLAGS) $(CFLAGSCURL) $<

curl_example.o: curl_example.c
	$(CC) $(CFLAGS) $(CFLAGSCURL) $<

custos_json_test.o: custos_json_test.c custos_json.h
	$(CC) $(CFLAGS) $(CFLAGSJSON) $<

custos_decode_test.o: custos_decode_test.c custos_json.h custos_http.h
	$(CC) $(CFLAGS) $(CFLAGSJSON) $<

../libb64-1.2.1/src/cdecode.o:
	$(MAKE) -C ../libb64-1.2.1 all_src

../libb64-1.2.1/src/cencode.o:
	$(MAKE) -C ../libb64-1.2.1 all_src

clean:
	rm -f $(ENCFS)
	rm -f $(MIRFS)
	rm -f $(FUSE_EXAMPLES)
	rm -f $(XATTR_EXAMPLES)
	rm -f $(OPENSSL_EXAMPLES)
	rm -f $(CURL_EXAMPLES)
	rm -f $(CUSTOS_TESTS)
	rm -f *.a
	rm -f *.o
	rm -f *~
	$(MAKE) -C ../libb64-1.2.1 clean_src
