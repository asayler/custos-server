#include <stdlib.h>
#include <stdio.h>

#include "http-util.h"

#define URI "http://condor.andysayler.com/test.json"

int main(int argc, char* argv[]) {

    (void) argc;
    (void) argv;

    HttpData_t res;
    long resCode;

    /* Initialize Data */
    res.size = 0;
    res.data = NULL;

    /* Make Request */
    resCode = httpGet(URI, &res);

    /* Print Output */
    fprintf(stdout, "resCode = %ld\n", resCode);
    fprintf(stdout, "res.size = %zd\n", res.size);
    if(res.data) {
	fprintf(stdout, "res.data:\n%s\n", res.data);
    }

    /* Clean Up */
    if(res.data) {
    	free(res.data);
    }

    return EXIT_SUCCESS;
}
