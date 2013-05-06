#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

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
	perror(         "--------------->");
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
    CurlData_t recData;

    recData.size = 0;    
    recData.data = NULL;

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

    ret = curl_easy_setopt(curl, CURLOPT_URL, "http://condor.andysayler.com/test.json");
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

    ret = curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    if(ret) {
	fprintf(stderr, "ERROR %s: curl_easy_setopt(CURLOPT_USERAGENT) failed - %s\n",
		argv[0], curl_easy_strerror(ret));
	goto EXIT_1;
    }

    ret = curl_easy_perform(curl);
    if(ret) {
	fprintf(stderr, "ERROR %s: curl_easy_perform() failed - %s\n",
		argv[0], curl_easy_strerror(ret));
    	goto EXIT_1;
    }
    
    recData.data = realloc(recData.data, (recData.size + 1));
    if(!(recData.data)) {
    	fprintf(stderr, "ERROR %s: realloc failed\n", argv[0]);
    	perror(         "--------------->");
    	goto EXIT_2;
    }
    recData.data[recData.size] = '\0';
    recData.size += 1;

    fprintf(stdout, "recData.size = %zd\n", recData.size);
    fprintf(stdout, "recData.data = %s\n", recData.data);

    if(recData.data) {
    	free(recData.data);
    }

    curl_easy_cleanup(curl);

    return EXIT_SUCCESS;

 EXIT_2:

    if(recData.data) {
	free(recData.data);
    }

 EXIT_1:

    curl_easy_cleanup(curl);    

 EXIT_0:
    
    return EXIT_FAILURE;

}
