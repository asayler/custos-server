#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "custos_http.h"
#include "json-util.h"

//#define URI "http://condor.andysayler.com/custos/01-deny-request.json"
//#define URI "http://condor.andysayler.com/custos/01-deny-response.json"
//#define URI "http://condor.andysayler.com/custos/01-accept-request.json"
#define URI "http://condor.andysayler.com/custos/01-accept-response.json"

int main(int argc, char* argv[]) {

    (void) argc;
    (void) argv;

    HttpData_t res;
    long resCode;
    json_tokener* tok;
    json_object* jobj;

    /* Initialize Data */
    res.size = 0;
    res.data = NULL;
    tok = NULL;
    jobj = NULL;

    /* Initialize Libs */
    if(httpInit() < 0) {
	fprintf(stderr, "httpInit() failed\n");
	exit(EXIT_FAILURE);
    }

    /* Make Request */
    resCode = httpGet(URI, &res);
    if(resCode != 200) {
	fprintf(stderr, "Failed to get URI: code %ld\n", resCode);
	exit(EXIT_FAILURE);
    }
    if(!res.data) {
	fprintf(stderr, "Failed to get response\n");
	exit(EXIT_FAILURE);
    }

    /* Decode JSON */
    tok = json_tokener_new();
    if(!tok) {
	fprintf(stderr, "Failed to create tokinizer\n");
	exit(EXIT_FAILURE);
    }
    jobj = json_tokener_parse_ex(tok, res.data, strlen(res.data));
    if(!jobj) {
	fprintf(stderr, "json_tokener_parse_ex failed\n");
	fprintf(stderr, "%d: %s\n", json_tokener_get_error(tok),
		json_tokener_error_desc(json_tokener_get_error(tok)));
	exit(EXIT_FAILURE);
    }


    /* Print output */
    if(jobj) {
	json_print(jobj, 0);
    }

    /* Clean Up */
    json_object_put(jobj);
    json_tokener_free(tok);
    free(res.data);

    /* Exit */
    return EXIT_SUCCESS;
}
