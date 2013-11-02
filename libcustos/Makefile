# File: Makefile
# By: Andy Sayler <www.andysayler.com>
# Build Custos Library

AR = ar
CC = gcc

CFLAGS  = -c -g -Wall -Wextra -Werror
LFLAGS  = -g -Wall -Wextra -Werror
ARFLAGS = rcsv

# Executables
CURL_EXAMPLES = curl_example
CUSTOS_TESTS  = custos_client_test custos_http_test custos_json_test custos_decode_test

# Static Libraries
CUSTOS_LIB         = libcustos.a
CUSTOS_LIB_OBJECTS = custos_client.o custos_client_print.o \
	             custos_http.o custos_json.o \
	             ../libb64-1.2.1/src/cdecode.o ../libb64-1.2.1/src/cencode.o

CFLAGSUUID    = `pkg-config uuid --cflags`
LLIBSUUID     = `pkg-config uuid --libs`
CFLAGSCURL    = `pkg-config libcurl --cflags`
LLIBSCURL     = `pkg-config libcurl --libs`
CFLAGSJSON    = `pkg-config json --cflags`
LLIBSJSON     = `pkg-config json --libs`
CFLAGSMHASH   =
LLIBSMHASH    = -lmhash

.PHONY: all libs tests examples clean \
	custos_lib custos_tests curl_examples

all: libs tests examples
libs: custos_lib
tests: custos_tests
examples: curl_examples

custos_lib: $(CUSTOS_LIB)
custos_tests: $(CUSTOS_TESTS)
curl_examples: $(CURL_EXAMPLES)


$(CUSTOS_LIB): $(CUSTOS_LIB_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

custos_client_test: custos_client_test.o $(CUSTOS_LIB)
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSCURL) $(LLIBSJSON) $(LLIBSUUID) $(LLIBSMHASH)

custos_decode_test: custos_decode_test.o $(CUSTOS_LIB)
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSJSON) $(LLIBSCURL) $(LLIBSMHASH)

custos_json_test: custos_json_test.o $(CUSTOS_LIB)
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSJSON)

custos_http_test: custos_http_test.o $(CUSTOS_LIB)
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSCURL) $(LLIBSMHASH)

curl_example: curl_example.o
	$(CC) $(LFLAGS) $^ -o $@ $(LLIBSCURL)

custos_client.o: custos_client.c custos_client.h
	$(CC) $(CFLAGS) $(CFLAGSUUID) $<

custos_client_print.o: custos_client_print.c custos_client_print.h custos_client.h
	$(CC) $(CFLAGS) $(CFLAGSUUID) $<

custos_http.o: custos_http.c custos_http.h
	$(CC) $(CFLAGS) $(CFLAGSCURL) $<

custos_json.o: custos_json.c custos_json.h
	$(CC) $(CFLAGS) $(CFLAGSJSON) $<

custos_client_test.o: custos_client_test.c custos_client.h custos_client_print.h
	$(CC) $(CFLAGS) $(CFLAGSUUID) $<

custos_http_test.o: custos_http_test.c custos_http.h
	$(CC) $(CFLAGS) $(CFLAGSCURL) $<

custos_json_test.o: custos_json_test.c custos_json.h
	$(CC) $(CFLAGS) $(CFLAGSJSON) $<

custos_decode_test.o: custos_decode_test.c custos_json.h custos_http.h
	$(CC) $(CFLAGS) $(CFLAGSJSON) $<

curl_example.o: curl_example.c
	$(CC) $(CFLAGS) $(CFLAGSCURL) $<

../libb64-1.2.1/src/cdecode.o:
	$(MAKE) -C ../libb64-1.2.1 all_src

../libb64-1.2.1/src/cencode.o:
	$(MAKE) -C ../libb64-1.2.1 all_src

clean:
	rm -f $(CURL_EXAMPLES)
	rm -f $(CUSTOS_TESTS)
	rm -f *.a
	rm -f *.o
	rm -f *~
	$(MAKE) -C ../libb64-1.2.1 clean_src
