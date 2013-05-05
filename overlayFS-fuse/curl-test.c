#include <stdlib.h>
#include <stdio.h>
#include <curl/curl.h>

int main(int argc, char* argv[]) {

    (void) argc;

    CURL *curl;
    CURLcode ret;

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

    ret = curl_easy_perform(curl);
    if(ret) {
	fprintf(stderr, "ERROR %s: curl_easy_perform() failed - %s\n",
		argv[0], curl_easy_strerror(ret));
    	goto EXIT_1;
    }
    
    curl_easy_cleanup(curl);

    return EXIT_SUCCESS;

 EXIT_1:

    curl_easy_cleanup(curl);    

 EXIT_0:
    
    return EXIT_FAILURE;

}
