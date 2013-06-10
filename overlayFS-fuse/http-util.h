#include <stdlib.h>

#include <curl/curl.h>

#define HTTP_USERAGENT "libcurl-agent/1.0"

typedef struct HttpData {
    char*  data;
    size_t size;
} HttpData_t;

long httpGet(const char* uri, HttpData_t* res);
