# File: Makefile
# By: Andy Sayler <www.andysayler.com>
# Build Custos Filesystem Demo

AR = ar
CC = gcc

CFLAGS  = -c -g -Wall -Wextra #-Werror
LFLAGS  = -g -Wall -Wextra #-Werror
ARFLAGS = rcsv

# Executables
ENCFS              = fuseenc fuseenc_fh
MIRFS              = fusemir_fh
FUSE_EXAMPLES      = fusehello fusexmp fusexmp_fh
XATTR_EXAMPLES     = xattr-util
OPENSSL_EXAMPLES   = aes-crypt-util
CURL_EXAMPLES      = curl_example
CUSTOS_TESTS       = custos_client_test custos_http_test custos_json_test custos_decode_test

CUSTOS_LIB         = ../../libcustos/libcustos.a

CFLAGSFUSE    = `pkg-config fuse --cflags`
LLIBSFUSE     = `pkg-config fuse --libs`
CFLAGSULOCK   =
LLIBSULOCK    = -lulockmgr
CFLAGSOPENSSL = `pkg-config openssl --cflags`
LLIBSOPENSSL  = `pkg-config openssl --libs`
CFLAGSUUID    = `pkg-config uuid --cflags`
LLIBSUUID     = `pkg-config uuid --libs`

.PHONY: all clean encfs mirfs fuse-examples xattr-examples openssl-examples

all: encfs mirfs fuse-examples xattr-examples openssl-examples

encfs: $(ENCFS)
mirfs: $(MIRFS)
fuse-examples: $(FUSE_EXAMPLES)
xattr-examples: $(XATTR_EXAMPLES)
openssl-examples: $(OPENSSL_EXAMPLES)

fusehello: fusehello.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSFUSE)

fusexmp: fusexmp.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSFUSE)

fusexmp_fh: fusexmp_fh.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSFUSE) $(LLIBSULOCK)

fuseenc: fuseenc.o aes-crypt.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSFUSE) $(LLIBSOPENSSL)

fuseenc_fh: fuseenc_fh.o aes-crypt.o $(CUSTOS_LIB)
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSFUSE) $(LLIBSULOCK) $(LLIBSOPENSSL) $(LLIBSUUID)

fusemir_fh: fusemir_fh.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSFUSE) $(LLIBSULOCK)

xattr-util: xattr-util.o
	$(CC) $(LFLAGS) $^ -o $@

aes-crypt-util: aes-crypt-util.o aes-crypt.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSOPENSSL)

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

clean:
	rm -f $(ENCFS)
	rm -f $(MIRFS)
	rm -f $(FUSE_EXAMPLES)
	rm -f $(XATTR_EXAMPLES)
	rm -f $(OPENSSL_EXAMPLES)
	rm -f *.a
	rm -f *.o
	rm -f *~
