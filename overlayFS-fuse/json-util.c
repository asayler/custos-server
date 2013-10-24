#include "json-util.h"

#define DEBUG

#define RETURN_SUCCESS 0

/*Parsing the json object*/
void json_print(json_object* jobj, int offset) {

    const json_type type = json_object_get_type(jobj);
    const char* sType = json_type_to_name(type);
    printf("%*s" "type: %s\n", offset, "", sType);
    printf("%*s" "val:  ", offset, "");

    switch(type) {
    case json_type_boolean:
	{
	    const int val = json_object_get_boolean(jobj);
	    printf("%d\n", val);
	}
	break;
    case json_type_double:
	{
	    const double val = json_object_get_double(jobj);
	    printf("%f\n", val);
	}
	break;
    case json_type_int:
	{
	    const int val = json_object_get_int(jobj);
	    printf("%d\n", val);
	}
	break;
    case json_type_string:
	{
	    const char* val = json_object_get_string(jobj);
	    printf("%s\n", val);
	}
	break;
    case json_type_object:
	{
	    json_object_iter iter;
	    printf("\n");
	    json_object_object_foreachC(jobj, iter) {
		const char* sKey  = iter.key;
		printf("%*s" "key:  %s\n", (offset +4), "", sKey);
		json_print(iter.val, (offset + 4));
	    }
	}
	break;
    case json_type_array:
	{
	    int len;
	    int i;
	    len = json_object_array_length(jobj);
	    printf("\n");
	    for(i=0; i < len; i++) {
		json_object* val = json_object_array_get_idx(jobj, i);
		json_print(val, (offset + 4));
	    }
	}
	break;
    case json_type_null:
	{
	    printf("WARNING - Null Type Detected/n");
	}
	break;
    }

    printf("%*s" "---\n", offset, "");

}

int json_safe_get(json_object* obj, json_type type, const char* key, void* val) {

    json_object* temp = NULL;

    /* Validate Args */
    if(!obj) {
#ifdef DEBUG
	fprintf(stderr, "ERROR json_safe_get: 'obj' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!json_object_is_type(obj, json_type_object)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR json_safe_get: 'obj' must be an object\n");
#endif
	return -EPERM;
    }
    if(!key) {
#ifdef DEBUG
	fprintf(stderr, "ERROR json_safe_get: 'key' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!val) {
#ifdef DEBUG
	fprintf(stderr, "ERROR json_safe_get: 'val' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Extract and Convert Value */
    if(!json_object_object_get_ex(obj, key, &temp)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR json_safe_get: key not found in object\n");
#endif
	return -EPERM;
    }
    if(!json_object_is_type(temp, type)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR json_safe_get: val of wrong type\n");
#endif
	return -EPERM;
    }
    switch(type) {
    case json_type_null:
	val = NULL;
	break;
    case json_type_object:
	*((json_object**) val) = temp;
	break;
    case json_type_int:
	*((int64_t*) val) = json_object_get_int64(temp);
	break;
    case json_type_string:
	*((char**) val) = strdup(json_object_get_string(temp));
	if(!(*((char**) val))) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR json_safe_get: strdup() failed\n");
#endif
	    return -EPERM;
	}
	break;
    default:
#ifdef DEBUG
	fprintf(stderr, "ERROR json_safe_get: unhandled type\n");
#endif
	return -EPERM;
	break;
    }

    /* Return */
    return RETURN_SUCCESS;

}
