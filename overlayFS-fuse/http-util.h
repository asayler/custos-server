#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <curl/curl.h>

#include "b64/cdecode.h"
#include "b64/cencode.h"

#define HTTP_USERAGENT "libcurl-agent/1.0"

typedef struct HttpData {
    char*  data;
    size_t size;
} HttpData_t;

int httpInit();
long httpGet(const char* uri, HttpData_t* res);

int encodeBase64(const char* data, const size_t data_size, char** text, size_t* text_size);
int decodeBase64(const char* text, const size_t text_size, char** data, size_t* data_size);
