#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "http-util.h"

#define DEBUG

#define RETURN_FAILURE -1
#define RETURN_SUCCESS  0

size_t writeCurlData(char* input, size_t size, size_t nmemb, void* output);

int httpInit() {

    CURLcode ret;

/* Initialize Curl */
/* Not thread safe */
    ret = curl_global_init(CURL_GLOBAL_SSL);
    if(ret) {
#ifdef DEBUG
	fprintf(stderr, "ERROR httpGet: curl_global_init failed - %s\n",
		curl_easy_strerror(ret));
#endif
	return RETURN_FAILURE;
    }

    return RETURN_SUCCESS;

}

long httpGet(const char* uri, HttpData_t* res) {

    CURLcode ret;
    CURL* curl;
    long resCode;

    curl = curl_easy_init();
    if(!curl) {
#ifdef DEBUG
	fprintf(stderr, "ERROR httpGet: curl_easy_init failed\n");
#endif
 	goto EXIT_0;
    }

    if(uri == NULL) {
#ifdef DEBUG
	fprintf(stderr, "ERROR httpGet: uri may not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Set Callback Options */
    ret = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCurlData);
    if(ret) {
#ifdef DEBUG
	fprintf(stderr, "ERROR httpGet: curl_easy_setopt(CURLOPT_WRITEFUNCTION) failed - %s\n",
		curl_easy_strerror(ret));
#endif
	goto EXIT_1;
    }
    ret = curl_easy_setopt(curl, CURLOPT_WRITEDATA, res);
    if(ret) {
#ifdef DEBUG
	fprintf(stderr, "ERROR httpGet: curl_easy_setopt(CURLOPT_WRITEDATA) failed - %s\n",
		curl_easy_strerror(ret));
#endif
	goto EXIT_1;
    }

    /* Set Request Options */
    ret = curl_easy_setopt(curl, CURLOPT_URL, uri);
    if(ret) {
#ifdef DEBUG
	fprintf(stderr, "ERROR httpGet: curl_easy_setopt(CURLOPT_URL) failed - %s\n",
		curl_easy_strerror(ret));
#endif
	goto EXIT_1;
    }
    ret = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    if(ret) {
#ifdef DEBUG
	fprintf(stderr, "ERROR httpGet: curl_easy_setopt(CURLOPT_FOLLOWLOCATION) failed - %s\n",
		curl_easy_strerror(ret));
#endif
	goto EXIT_1;
    }
    ret = curl_easy_setopt(curl, CURLOPT_USERAGENT, HTTP_USERAGENT);
    if(ret) {
#ifdef DEBUG
	fprintf(stderr, "ERROR httpGet: curl_easy_setopt(CURLOPT_USERAGENT) failed - %s\n",
		curl_easy_strerror(ret));
#endif
	goto EXIT_1;
    }

    /* Make Request */
    ret = curl_easy_perform(curl);
    if(ret) {
#ifdef DEBUG
	fprintf(stderr, "ERROR httpGet: curl_easy_perform() failed - %s\n",
		curl_easy_strerror(ret));
#endif
    	goto EXIT_1;
    }

    /* Get Return Status */
    ret = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resCode);
    if(ret) {
#ifdef DEBUG
	fprintf(stderr, "ERROR httpGet: curl_easy_getinfo(CURLINFO_RESPONSE_CODE) failed - %s\n",
		curl_easy_strerror(ret));
#endif
    	goto EXIT_1;
    }

    /* Add Null Terminators to Data */
    res->data = realloc(res->data, (res->size + 1));
    if(!(res->data)) {
#ifdef DEBUG
    	fprintf(stderr, "ERROR httpGet: realloc failed\n");
    	perror(         "--------------->");
#endif
    	goto EXIT_2;
    }
    res->data[res->size] = '\0';
    res->size += 1;

    /* Clean Up */
    curl_easy_cleanup(curl);

    return resCode;

 EXIT_2:

    /* Do Nothing...*/

 EXIT_1:

    curl_easy_cleanup(curl);

 EXIT_0:

    return RETURN_FAILURE;

}

size_t writeCurlData(char* input, size_t size, size_t nmemb, void* output) {

    HttpData_t* outData = output;
    size_t fullSize = size * nmemb;

    outData->data = realloc(outData->data, (outData->size + fullSize));
    if(!(outData->data)) {
	fprintf(stderr, "ERROR writeCurlData: realloc failed\n");
	perror(         "------------------->");
	return 0;
    }

    memcpy(&(outData->data[outData->size]), input, fullSize);
    outData->size += fullSize;

    return fullSize;

}
