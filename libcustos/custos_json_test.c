#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "custos_json.h"

int main(int argc, char* argv[]) {

    (void) argc;
    (void) argv;

    char* string =
	"{ "
	"\"sitename\": "
	"\"joys of programming\", "
	"\"categories\": "
	"[ \"c\", [ \"c++\", \"c\" ], \"java\", \"PHP\" ], "
	"\"author-details\": "
	"{ \"admin\": false, \"name\": \"Joys of Programming\", \"Number of Posts\": 10 }"
	" }";

    printf("input: %s\n", string);

    json_object* jobj = json_tokener_parse(string);

    printf("jobj:  %s\n", json_object_to_json_string(jobj));

    json_print(jobj, 0);

    json_object_put(jobj);

    return EXIT_SUCCESS;

}
