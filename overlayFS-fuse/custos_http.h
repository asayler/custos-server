#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <stdbool.h>

#include <curl/curl.h>
#include <mhash.h>

#include "b64/cdecode.h"
#include "b64/cencode.h"

#define HTTP_USERAGENT "libcurl-agent/1.0"

typedef struct HttpData {
    char*  data;
    size_t size;
} HttpData_t;

int httpInit();
long httpGet(const char* uri, HttpData_t* res);

int encodeBase64(const char* decoded, const size_t decodedSize, char** encoded, size_t* encodedSize);
int decodeBase64(const char* encoded, const size_t encodedSize, char** decoded, size_t* decodedSize);
int freeBase64(char** value);

int encodeURL(const char* decoded, const size_t decodedSize, char** encoded, size_t* encodedSize);
int decodeURL(const char* encoded, const size_t encodedSize, char** decoded, size_t* decodedSize);
int freeURL(char** value);

char* hashMD5(const char* value, const size_t size);
int freeHash(char** value);

char* buildUrlGet(char* target, char* endpoint, ...);
