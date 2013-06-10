#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <json/json.h>

#include "json-util.h"

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
