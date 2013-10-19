#include "http-util.h"

#define DEBUG

#define RETURN_FAILURE -1
#define RETURN_SUCCESS  0

#define B64_LINE_LENGTH 72

size_t writeCurlData(char* input, size_t size, size_t nmemb, void* output);

int encodeBase64(const char* decoded, const size_t decodedSize,
		 char** encoded, size_t* encodedSize) {

    /* Local Args and Inits */
    int    cnt      = 0;
    size_t total    = 0;
    char*  itr      = NULL;
    size_t padSize  = (decodedSize % 3) ? (3 - (decodedSize % 3)) : (0);
    size_t ceilSize = decodedSize + padSize;
    size_t encSize  = (ceilSize * 4) / 3;
    size_t eolSize  = (encSize / B64_LINE_LENGTH) + 1;
    base64_encodestate s;

    /* Verify Required Arguments */
    if(!encoded) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeBase64: 'encoded' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(*encoded) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeBase64: '*encoded' must be NULL\n");
#endif
	return -EINVAL;
    }
    if(!encodedSize) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeBase64: 'encodedSize' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(*encodedSize) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeBase64: '*encodedSize' must be 0\n");
#endif
	return -EINVAL;
    }

    /* Handle Null Case */
    if(!decodedSize) {
	return RETURN_SUCCESS;
    }

    /* Verify Optional Arguments */
    if(!decoded) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeBase64: 'decoded' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Malloc Space */
    *encodedSize = encSize + eolSize + 1;
    *encoded = itr = malloc(*encodedSize);
    if(!(*encoded)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeBase64: malloc() failed\n");
	perror(         "------------------>");
#endif
	return -errno;
    }

    /* Perform  Encoding */
    base64_init_encodestate(&s);
    cnt = base64_encode_block(decoded, decodedSize, itr, &s);
    if(cnt < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeBase64: base64_encode_block() failed\n");
#endif
	return RETURN_FAILURE;
    }
    itr += cnt;
    total += cnt;
    cnt = base64_encode_blockend(itr, &s);
    if(cnt < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeBase64: base64_encode_blockend() failed\n");
#endif
	return RETURN_FAILURE;
    }
    itr += cnt;
    total += cnt;

    /* Add Null Terminator */
    *itr = '\0';
    total += 1;

    return total;

}

int decodeBase64(const char* encoded, const size_t encodedSize,
		 char** decoded, size_t* decodedSize) {

    /* Local Args and Inits */
    int    cnt      = 0;
    size_t total    = 0;
    char*  itr      = NULL;
    size_t decSize  = (encodedSize * 3) / 4;
    base64_decodestate s;

    /* Verify Required Arguments */
    if(!decoded) {
#ifdef DEBUG
	fprintf(stderr, "ERROR decodeBase64: 'decoded' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(*decoded) {
#ifdef DEBUG
	fprintf(stderr, "ERROR decodeBase64: '*decoded' must be NULL\n");
#endif
	return -EINVAL;
    }
    if(!decodedSize) {
#ifdef DEBUG
	fprintf(stderr, "ERROR decodeBase64: 'decodedSize' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(*decodedSize) {
#ifdef DEBUG
	fprintf(stderr, "ERROR decodeBase64: '*decodedSize' must be 0\n");
#endif
	return -EINVAL;
    }

    /* Handle Null Case */
    if(!encodedSize) {
	return RETURN_SUCCESS;
    }

    /* Verify Optional Arguments */
    if(!encoded) {
#ifdef DEBUG
	fprintf(stderr, "ERROR decodeBase64: 'encoded' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Malloc Space */
    *decodedSize = decSize;
    *decoded = itr = malloc(*decodedSize);
    if(!(*decoded)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR decodeBase64: malloc() failed\n");
	perror(         "------------------>");
#endif
	return -errno;
    }

    /* Perform Decoding */
    base64_init_decodestate(&s);
    cnt = base64_decode_block(encoded, encodedSize, itr, &s);
    if(cnt < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR decodeBase64: base64_decode_block() failed\n");
#endif
	return RETURN_FAILURE;
    }
    itr += cnt;
    total += cnt;

    return total;

}

int freeBase64(char** value) {

    /* Verify Required Arguments */
    if(!value) {
#ifdef DEBUG
	fprintf(stderr, "ERROR freeBase64: value must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!(*value)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR freeBase64: *value must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Free Data */
    free(*value);
    *value = NULL;

    return EXIT_SUCCESS;

}

int encodeURL(const char* decoded, const size_t decodedSize,
	      char** encoded, size_t* encodedSize) {

    /* Local Args and Inits */
    CURL* curl;

    /* Verify Required Arguments */
    if(!encoded) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeURL: 'encoded' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(*encoded) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeURL: '*encoded' must be NULL\n");
#endif
	return -EINVAL;
    }
    if(!encodedSize) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeURL: 'encodedSize' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(*encodedSize) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeURL: '*encodedSize' must be 0\n");
#endif
	return -EINVAL;
    }

    /* Handle Null Case */
    if(!decodedSize) {
	return RETURN_SUCCESS;
    }

    /* Verify Optional Arguments */
    if(!decoded) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeURL: 'decoded' must not be NULL\n");
#endif
	return -EINVAL;
    }

    curl = curl_easy_init();
    if(!curl) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeURL: curl_easy_init() failed\n");
#endif
	return RETURN_FAILURE;
    }

    *encoded = curl_easy_escape(curl, decoded, decodedSize);
    if(!*(encoded)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR encodeURL: curl_easy_escape() failed\n");
#endif
	return RETURN_FAILURE;
    }

    *encodedSize = strlen(*encoded);

    return RETURN_SUCCESS;

}

int decodeURL(const char* encoded, const size_t encodedSize,
	      char** decoded, size_t* decodedSize) {

    /* Local Args and Inits */
    CURL* curl;

    /* Verify Required Arguments */
    if(!decoded) {
#ifdef DEBUG
	fprintf(stderr, "ERROR decodeURL: 'decoded' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(*decoded) {
#ifdef DEBUG
	fprintf(stderr, "ERROR decodeURL: '*decoded' must be NULL\n");
#endif
	return -EINVAL;
    }
    if(!decodedSize) {
#ifdef DEBUG
	fprintf(stderr, "ERROR decodeURL: 'decodedSize' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(*decodedSize) {
#ifdef DEBUG
	fprintf(stderr, "ERROR decodeURL: '*decodedSize' must be 0\n");
#endif
	return -EINVAL;
    }

    /* Handle Null Case */
    if(!encodedSize) {
	return RETURN_SUCCESS;
    }

    /* Verify Optional Arguments */
    if(!encoded) {
#ifdef DEBUG
	fprintf(stderr, "ERROR decodeURL: 'encoded' must not be NULL\n");
#endif
	return -EINVAL;
    }

    curl = curl_easy_init();
    if(!curl) {
#ifdef DEBUG
	fprintf(stderr, "ERROR decodeURL: curl_easy_init() failed\n");
#endif
	return RETURN_FAILURE;
    }

    *decoded = curl_easy_unescape(curl, encoded, encodedSize, (int*) decodedSize);
    if(!*(decoded)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR decodeURL: curl_easy_escape() failed\n");
#endif
	return RETURN_FAILURE;
    }

    return RETURN_SUCCESS;

}

int freeURL(char** value) {

   /* Verify Required Arguments */
    if(!value) {
#ifdef DEBUG
	fprintf(stderr, "ERROR freeBase64: value must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!(*value)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR freeBase64: *value must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Free Data */
    curl_free(*value);
    *value = NULL;

    return EXIT_SUCCESS;

}


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
