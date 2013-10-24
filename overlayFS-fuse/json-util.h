#ifndef JSON_UTIL_H
#define JSON_UTIL_H

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <json/json.h>

void json_print(json_object* jobj, int offset);

int json_safe_get(json_object* obj, json_type type, const char* key, void* val);

#endif
