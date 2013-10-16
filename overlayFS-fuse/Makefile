# File: Makefile
# By: Andy Sayler <www.andysayler.com>
# Build Custos Filesystem Demo

CC           = gcc

CFLAGSFUSE    = `pkg-config fuse --cflags`
LLIBSFUSE     = `pkg-config fuse --libs`
LLIBSULOCK    = -lulockmgr
CFLAGSOPENSSL = `pkg-config openssl --cflags`
LLIBSOPENSSL  = `pkg-config openssl --libs`
CFLAGSUUID    = `pkg-config uuid --cflags`
LLIBSUUID     = `pkg-config uuid --libs`
CFLAGSCURL    = `pkg-config libcurl --cflags`
LLIBSCURL     = `pkg-config libcurl --libs`
CFLAGSJSON    = `pkg-config json --cflags`
LLIBSJSON     = `pkg-config json --libs`

CFLAGS = -c -g -Wall -Wextra -Werror
LFLAGS = -g -Wall -Wextra -Werror

ENCFS            = fuseenc fuseenc_fh
MIRFS            = fusemir_fh
FUSE_EXAMPLES    = fusehello fusexmp fusexmp_fh
XATTR_EXAMPLES   = xattr-util
OPENSSL_EXAMPLES = aes-crypt-util
CUSTOS_UTIL      = custos_client_util
HTTP_TEST        = http-test
CURL_TEST        = curl-test
JSON_TEST        = json-test
DECODE_TEST      = decode-test
CUSTOS_TEST      = custos_client_test

.PHONY: all tests encfs mirfs \
	fuse-examples xattr-examples openssl-examples \
	custos_test http_test curl_test json_test decode_test \
	clean

all: encfs mirfs \
	fuse-examples xattr-examples openssl-examples \
	custos_test http_test curl_test json_test decode_test

tests: custos_test http_test curl_test json_test decode_test

encfs: $(ENCFS)
mirfs: $(MIRFS)
fuse-examples: $(FUSE_EXAMPLES)
xattr-examples: $(XATTR_EXAMPLES)
openssl-examples: $(OPENSSL_EXAMPLES)
http_test: $(HTTP_TEST)
curl_test: $(CURL_TEST)
json_test: $(JSON_TEST)
decode_test: $(DECODE_TEST)
custos_test: $(CUSTOS_TEST)

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

custos_client_test: custos_client_test.o custos_client.o custos_client_print.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSCURL) $(LLIBSJSON) $(LLIBSUUID)

http-test: http-test.o http-util.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSCURL)

curl-test: curl-test.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSCURL)

json-test: json-test.o json-util.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSJSON)

decode-test: decode-test.o json-util.o http-util.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSJSON) $(LLIBSCURL)

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

http-util.o: http-util.c http-util.h
	$(CC) $(CFLAGS) $(CFLAGSCURL) $<

json-util.o: json-util.c json-util.h
	$(CC) $(CFLAGS) $(CFLAGSJSON) $<

http-test.o: http-test.c http-util.h
	$(CC) $(CFLAGS) $(CFLAGSCURL) $<

curl-test.o: curl-test.c
	$(CC) $(CFLAGS) $(CFLAGSCURL) $<

json-test.o: json-test.c json-util.h
	$(CC) $(CFLAGS) $(CFLAGSJSON) $<

decode-test.o: decode-test.c json-util.h http-util.h
	$(CC) $(CFLAGS) $(CFLAGSJSON) $<

clean:
	rm -f $(ENCFS)
	rm -f $(MIRFS)
	rm -f $(FUSE_EXAMPLES)
	rm -f $(XATTR_EXAMPLES)
	rm -f $(OPENSSL_EXAMPLES)
	rm -f $(CUSTOS_TEST)
	rm -f $(HTTP_TEST)
	rm -f $(CURL_TEST)
	rm -f $(JSON_TEST)
	rm -f $(DECODE_TEST)
	rm -f *.o
	rm -f *~
	rm -f handout/*~
	rm -f handout/*.log
	rm -f handout/*.aux
	rm -f handout/*.out
