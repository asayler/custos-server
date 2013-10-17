#include "http-util.h"

#define DEBUG

#define RETURN_FAILURE -1
#define RETURN_SUCCESS  0

#define B64_LINE_LENGTH 72

int encodeBase64(const char* data, const size_t dataSize,
		 char** text, size_t* textSize) {

    int    cnt      = 0;
    size_t total    = 0;
    char*  itr      = NULL;
    size_t padSize  = (dataSize % 3) ? (3 - (dataSize % 3)) : (0);
    size_t ceilSize = dataSize + padSize;
    size_t encSize  = (ceilSize * 4) / 3;
    size_t eolSize  = (encSize / B64_LINE_LENGTH) + 1;
    base64_encodestate s;

    *textSize = encSize + eolSize + 1;
    *text = itr = malloc(*textSize);
    if(!(*text)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeBase64: malloc() failed\n");
	perror(         "------------------>");
#endif
	return -errno;
    }

    /*---------- START ENCODING ----------*/
    /* initialise the encoder state */
    base64_init_encodestate(&s);
    /* gather data from the input and send it to the output */
    cnt = base64_encode_block(data, dataSize, itr, &s);
    if(cnt < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeBase64: base64_encode_block() failed\n");
#endif
	return RETURN_FAILURE;
    }
    itr += cnt;
    total += cnt;
    /* Finalise the encoding */
    cnt = base64_encode_blockend(itr, &s);
    if(cnt < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeBase64: base64_encode_blockend() failed\n");
#endif
	return RETURN_FAILURE;
    }
    itr += cnt;
    total += cnt;
    /*---------- STOP ENCODING  ----------*/

    /* Add Null Terminator */
    *itr = '\0';
    total += 1;

    return total;

}

int decodeBase64(const char* text, const size_t textSize,
		 char** data, size_t* dataSize) {

    int    cnt      = 0;
    size_t total    = 0;
    char*  itr      = NULL;
    size_t decSize  = (textSize * 3) / 4;
    base64_decodestate s;

    *dataSize = decSize;
    *data = itr = malloc(*dataSize);
    if(!(*data)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR decodeBase64: malloc() failed\n");
	perror(         "------------------>");
#endif
	return -errno;
    }

    /*---------- START DECODING ----------*/
    /* initialise the decoder state */
    base64_init_decodestate(&s);
    /* Decode data */
    cnt = base64_decode_block(text, textSize, itr, &s);
    if(cnt < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR decodeBase64: base64_decode_block() failed\n");
#endif
	return RETURN_FAILURE;
    }
    itr += cnt;
    total += cnt;
    /*---------- STOP ENCODING  ----------*/

    return total;

}

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
    ret = curl_easy_setopt(curl, CURLOPT_HTTPGET, true);
    if(ret) {
#ifdef DEBUG
	fprintf(stderr, "ERROR httpGet: curl_easy_setopt(CURLOPT_HTTPGET) failed - %s\n",
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
#ifdef DEBUG
	fprintf(stderr, "ERROR writeCurlData: realloc failed\n");
	perror(         "------------------->");
#endif
	return 0;
    }

    memcpy(&(outData->data[outData->size]), input, fullSize);
    outData->size += fullSize;

    return fullSize;

}
