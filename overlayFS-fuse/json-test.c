#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <json/json.h>

/*Parsing the json object*/
void json_parse(json_object* jobj) {

    json_object_iter iter;

    json_object_object_foreachC(jobj, iter) {

	const json_type type = json_object_get_type(iter.val);
	const char* sType = json_type_to_name(type);
	const char* sKey  = iter.key;

	printf("type: %s\n", sType);

	switch(type) {
	case json_type_boolean:
	    {
		const int val = json_object_get_boolean(iter.val);
		printf("key:  %s\n", sKey);
		printf("val:  %d\n", val);
	    }
	    break;
	case json_type_double:
	    {
		const double val = json_object_get_double(iter.val);
		printf("key:  %s\n", sKey);
		printf("val:  %f\n", val);
	    }
	    break;
	case json_type_int:
	    {
		const int val = json_object_get_int(iter.val);
		printf("key:  %s\n", sKey);
		printf("val:  %d\n", val);
	    }
	    break;
	case json_type_string:
	    {
		const char* val = json_object_get_string(iter.val);
		printf("key:  %s\n", sKey);
		printf("val:  %s\n", val);
	    }
	    break;
	case json_type_object:
	    {
		jobj = json_object_object_get(jobj, iter.key);
		printf("key:  %s\n", sKey);
		json_parse(jobj);
	    }
	    break;
	case json_type_array:
	    {
		printf("key:  %s\n", sKey);
	    }
	    break;
	case json_type_null:
	    printf("WARNING - Null Type Detected/n");
	    break;

	}

    }

}

int main(int argc, char* argv[]) {

    (void) argc;
    (void) argv;

    char* string = 
	"{"
	"\"sitename\" : "
	"\"joys of programming\","
	"\"categories\" : "
	"[ \"c\" , [\"c++\" , \"c\" ], \"java\", \"PHP\" ],"
	"\"author-details\": "
	"{ \"admin\": false, \"name\" : \"Joys of Programming\", \"Number of Posts\" : 10 }"
	"}";

    printf("JSON string: %s\n", string);
    
    json_object* jobj = json_tokener_parse(string);
    json_parse(jobj);
        
    return EXIT_SUCCESS;

}
