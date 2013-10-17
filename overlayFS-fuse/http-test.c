#include <stdlib.h>
#include <stdio.h>

#include "http-util.h"

#define URI "http://condor.andysayler.com/custos/01-accept-response.json"

int main(int argc, char* argv[]) {

    (void) argc;
    (void) argv;

    HttpData_t res;
    long resCode;

    char input[] = "This is a test. This is another test. This is a third test.";
    char* text = NULL;
    size_t textSize = 0;
    char* data = NULL;
    size_t dataSize = 0;
    int ret;

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

    /* Base 64 Encode */
    fprintf(stdout, "\n*** Test encodeBase64() ***\n");
    fprintf(stdout, "input:\n%s\n", input);
    ret = encodeBase64(input, sizeof(input), &text, &textSize);
    if (ret < 0) {
	fprintf(stderr, "encodeBase64 failed\n");
	exit(EXIT_FAILURE);
    }
    fprintf(stdout, "ret      = %d\n",   ret);
    fprintf(stdout, "textSize = %zd\n",  textSize);
    fprintf(stdout, "text:\n%s", text);

    /* Base 64 Decode */
    fprintf(stdout, "\n*** Test decodeBase64() ***\n");
    fprintf(stdout, "input:\n%s", text);
    ret = decodeBase64(text, textSize, &data, &dataSize);
    if (ret < 0) {
	fprintf(stderr, "decodeBase64 failed\n");
	exit(EXIT_FAILURE);
    }
    fprintf(stdout, "ret      = %d\n",   ret);
    fprintf(stdout, "dataSize = %zd\n",  dataSize);
    fprintf(stdout, "data:\n%s\n", data);

    if(ret != sizeof(input)) {
	fprintf(stderr, "Base64 input size does not match Base64 output size!\n");
	exit(EXIT_FAILURE);
    }

    if(strncmp(input, data, dataSize)) {
	fprintf(stderr, "Base64 input content does not match Base64 output content!\n");
	exit(EXIT_FAILURE);
    }

    /* Clean Up */
    if(text) {
	free(text);
    }
    if(data) {
	free(data);
    }

    return EXIT_SUCCESS;
}
