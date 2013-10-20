#include <stdlib.h>
#include <stdio.h>

#include "http-util.h"

#define URI "http://condor.andysayler.com/custos/01-accept-response.json"

int main(int argc, char* argv[]) {

    (void) argc;
    (void) argv;

    HttpData_t res;
    long resCode;

    char base64Input[] = "This is a test. This is another test. This is a third test.";
    char urlInput[] = "{\"Target\": \"http:\\/\\/test.com\", \"Version\": \"0.1-dev\"}";
    char hashInput[] = "This is a test of your hashing function";
    char* encoded = NULL;
    size_t encodedSize = 0;
    char* decoded = NULL;
    size_t decodedSize = 0;
    char* hash = NULL;
    size_t hashSize = 0;
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

    /* Base64 Encode */
    fprintf(stdout, "\n*** Test encodeBase64() ***\n");
    fprintf(stdout, "input:\n%s\n", base64Input);
    ret = encodeBase64(base64Input, sizeof(base64Input), &encoded, &encodedSize);
    if(ret < 0) {
    	fprintf(stderr, "encodeBase64 failed\n");
    	exit(EXIT_FAILURE);
    }
    fprintf(stdout, "ret         = %d\n",   ret);
    fprintf(stdout, "encodedSize = %zd\n",  encodedSize);
    fprintf(stdout, "encoded:\n%s",         encoded);

    /* Base64 Decode */
    fprintf(stdout, "\n*** Test decodeBase64() ***\n");
    fprintf(stdout, "input:\n%s", encoded);
    ret = decodeBase64(encoded, encodedSize, &decoded, &decodedSize);
    if(ret < 0) {
    	fprintf(stderr, "decodeBase64 failed\n");
    	exit(EXIT_FAILURE);
    }
    fprintf(stdout, "ret         = %d\n",  ret);
    fprintf(stdout, "decodedSize = %zd\n", decodedSize);
    fprintf(stdout, "decoded:\n%s\n"     , decoded);

    /* Base64 Check */
    if(ret != sizeof(base64Input)) {
    	fprintf(stderr, "Base64 input size does not match Base64 output size!\n");
    	exit(EXIT_FAILURE);
    }
    if(strncmp(base64Input, decoded, decodedSize)) {
    	fprintf(stderr, "Base64 input content does not match Base64 output content!\n");
    	exit(EXIT_FAILURE);
    }

    /* Clean Up Base64 */
    if(encoded) {
    	if(freeBase64(&encoded) < 0) {
	    fprintf(stderr, "freeBase64() failed!\n");
	    exit(EXIT_FAILURE);
	}
    	encodedSize = 0;
    }
    if(decoded) {
    	if(freeBase64(&decoded) < 0) {
	    fprintf(stderr, "freeBase64() failed!\n");
	    exit(EXIT_FAILURE);
	}
    	decodedSize = 0;
    }

    /* URL Encode */
    fprintf(stdout, "\n*** Test encodeURL64() ***\n");
    fprintf(stdout, "input:\n%s\n", urlInput);
    ret = encodeURL(urlInput, strlen(urlInput), &encoded, &encodedSize);
    if(ret < 0) {
    	fprintf(stderr, "encodeURL failed\n");
    	exit(EXIT_FAILURE);
    }
    fprintf(stdout, "ret         = %d\n",   ret);
    fprintf(stdout, "encodedSize = %zd\n",  encodedSize);
    fprintf(stdout, "encoded:\n%s\n",       encoded);

    /* URL Decode */
    fprintf(stdout, "\n*** Test decodeURL64() ***\n");
    fprintf(stdout, "input:\n%s\n", encoded);
    ret = decodeURL(encoded, encodedSize, &decoded, &decodedSize);
    if(ret < 0) {
    	fprintf(stderr, "decodeURL failed\n");
    	exit(EXIT_FAILURE);
    }
    fprintf(stdout, "ret         = %d\n",   ret);
    fprintf(stdout, "decodedSize = %zd\n",  decodedSize);
    fprintf(stdout, "decoded:\n%s\n",       decoded);

    /* URL Check */
    if(ret != (int) strlen(urlInput)) {
    	fprintf(stderr, "URL input size does not match URL output size!\n");
    	exit(EXIT_FAILURE);
    }
    if(strncmp(urlInput, decoded, decodedSize)) {
    	fprintf(stderr, "URL input content does not match URL output content!\n");
    	exit(EXIT_FAILURE);
    }

    /* Clean Up URL */
    if(encoded) {
    	if(freeURL(&encoded) < 0) {
	    fprintf(stderr, "freeURL() failed!\n");
	    exit(EXIT_FAILURE);
	}
    	encodedSize = 0;
    }
    if(decoded) {
    	if(freeURL(&decoded) < 0) {
	    fprintf(stderr, "freeURL() failed!\n");
	    exit(EXIT_FAILURE);
	}
    	decodedSize = 0;
    }

    /* MD5 Check */
    fprintf(stdout, "input = %s\n", hashInput);
    hash = hashMD5(hashInput, 0);
    if(!hash) {
    	fprintf(stderr, "hashMD5() failed\n");
    	exit(EXIT_FAILURE);
    }
    hashSize = strlen(hash);
    fprintf(stdout, "hashSize = %zd\n", hashSize);
    fprintf(stdout, "hash md5 = %s\n", hash);

    /* Clean up MD5 */
    if(hash) {
    	if(freeHash(&hash) < 0) {
	    fprintf(stderr, "freeHash() failed!\n");
	    exit(EXIT_FAILURE);
	}
    	hashSize = 0;
    }

    return EXIT_SUCCESS;
}
