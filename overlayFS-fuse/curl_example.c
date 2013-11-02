#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#define URI "http://condor.andysayler.com/custos/01-accept-response.json"
#define USERAGENT "libcurl-agent/1.0"

typedef struct CurlData {
    char*  data;
    size_t size;
} CurlData_t;

size_t writeCurlData(char* input, size_t size, size_t nmemb, void* output) {

    CurlData_t* outData = output;
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

int main(int argc, char* argv[]) {

    (void) argc;

    CURL* curl;
    CURLcode ret;
    CurlData_t recHeader;
    CurlData_t recData;
    long resCode;

    /* Initialize Data */
    recHeader.size = 0;
    recHeader.data = NULL;
    recData.size = 0;
    recData.data = NULL;

    /* Initialize Curl */
    ret = curl_global_init(CURL_GLOBAL_SSL);
    if(ret) {
	fprintf(stderr, "ERROR %s: curl_global_init failed - %s\n",
		argv[0], curl_easy_strerror(ret));
	goto EXIT_0;
    }
    curl = curl_easy_init();
    if(!curl) {
	fprintf(stderr, "ERROR %s: curl_easy_init failed - %s\n",
		argv[0], curl_easy_strerror(ret));
       	goto EXIT_0;
    }

    /* Set Callback Options */
    ret = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCurlData);
    if(ret) {
	fprintf(stderr, "ERROR %s: curl_easy_setopt(CURLOPT_WRITEFUNCTION) failed - %s\n",
		argv[0], curl_easy_strerror(ret));
	goto EXIT_1;
    }
    ret = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &recData);
    if(ret) {
	fprintf(stderr, "ERROR %s: curl_easy_setopt(CURLOPT_WRITEDATA) failed - %s\n",
		argv[0], curl_easy_strerror(ret));
	goto EXIT_1;
    }
    ret = curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &recHeader);
    if(ret) {
	fprintf(stderr, "ERROR %s: curl_easy_setopt(CURLOPT_WRITEHEADER) failed - %s\n",
		argv[0], curl_easy_strerror(ret));
	goto EXIT_1;
    }

    /* Set Request Options */
    ret = curl_easy_setopt(curl, CURLOPT_URL, URI);
    if(ret) {
	fprintf(stderr, "ERROR %s: curl_easy_setopt(CURLOPT_URL) failed - %s\n",
		argv[0], curl_easy_strerror(ret));
	goto EXIT_1;
    }
    ret = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    if(ret) {
	fprintf(stderr, "ERROR %s: curl_easy_setopt(CURLOPT_FOLLOWLOCATION) failed - %s\n",
		argv[0], curl_easy_strerror(ret));
	goto EXIT_1;
    }
    ret = curl_easy_setopt(curl, CURLOPT_USERAGENT, USERAGENT);
    if(ret) {
	fprintf(stderr, "ERROR %s: curl_easy_setopt(CURLOPT_USERAGENT) failed - %s\n",
		argv[0], curl_easy_strerror(ret));
	goto EXIT_1;
    }

    /* Make Request */
    ret = curl_easy_perform(curl);
    if(ret) {
	fprintf(stderr, "ERROR %s: curl_easy_perform() failed - %s\n",
		argv[0], curl_easy_strerror(ret));
    	goto EXIT_1;
    }

    /* Get Return Status */
    ret = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resCode);
    if(ret) {
	fprintf(stderr, "ERROR %s: curl_easy_getinfo(CURLINFO_RESPONSE_CODE) failed - %s\n",
		argv[0], curl_easy_strerror(ret));
    	goto EXIT_1;
    }

    /* Add Null Terminators to Data */
    recHeader.data = realloc(recHeader.data, (recHeader.size + 1));
    if(!(recHeader.data)) {
    	fprintf(stderr, "ERROR %s: realloc failed\n", argv[0]);
    	perror(         "--------------->");
    	goto EXIT_2;
    }
    recHeader.data[recHeader.size] = '\0';
    recHeader.size += 1;

    recData.data = realloc(recData.data, (recData.size + 1));
    if(!(recData.data)) {
    	fprintf(stderr, "ERROR %s: realloc failed\n", argv[0]);
    	perror(         "--------------->");
    	goto EXIT_2;
    }
    recData.data[recData.size] = '\0';
    recData.size += 1;

    /* Print Output */
    fprintf(stdout, "resCode = %ld\n", resCode);
    fprintf(stdout, "recHeader.size = %zd\n", recHeader.size);
    fprintf(stdout, "recHeader.data:\n%s\n", recHeader.data);
    fprintf(stdout, "recData.size = %zd\n", recData.size);
    fprintf(stdout, "recData.data:\n%s\n", recData.data);

    /* Clean Up */
    if(recHeader.data) {
    	free(recHeader.data);
    }
    if(recData.data) {
    	free(recData.data);
    }
    curl_easy_cleanup(curl);

    return EXIT_SUCCESS;

 EXIT_2:

    if(recHeader.data) {
	free(recHeader.data);
    }

    if(recData.data) {
	free(recData.data);
    }

 EXIT_1:

    curl_easy_cleanup(curl);

 EXIT_0:

    return EXIT_FAILURE;

}
