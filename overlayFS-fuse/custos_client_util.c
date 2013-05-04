/* custos_client_util.c
 * 
 * custos_client utility tool
 *
 * By Andy Sayler (www.andysayler.com)
 * Created  05/02/13
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "custos_client.h"

int main(int argc, char* argv[]) {

    (void) argc;
    (void) argv;

    int ret;
    uuid_t uuid;
    custosKeyReq_t* req;
    char* pw = "password";

    uuid_generate(uuid);
    req = custos_createKeyReq(uuid, "http://test.com");
    if(!req) {
	fprintf(stderr, "ERROR %s: custos_createKeyReq failed\n", argv[0]);
	return EXIT_FAILURE;
    }
    
    ret = custos_updateKeyReq(req, CUS_ATTRID_PSK, pw, (strlen(pw) + 1));
    if(ret < 0) {
    	fprintf(stderr, "ERROR %s: custos_updateKeyReq failed\n", argv[0]);
    	return EXIT_FAILURE;
    }
    ret = custos_updateKeyReq(req, CUS_ATTRID_PSK, pw, (strlen(pw) + 1));
    if(ret < 0) {
    	fprintf(stderr, "ERROR %s: custos_updateKeyReq failed\n", argv[0]);
    	return EXIT_FAILURE;
    }

    ret = custos_destroyKeyReq(&req);
    if(ret < 0) {
	fprintf(stderr, "ERROR %s: custos_destroyKeyReq failed\n", argv[0]);
	return EXIT_FAILURE;
    }
    if(req) {
	fprintf(stderr, "ERROR %s: custos_destroyKeyReq failed to set req to NULL\n", argv[0]);
	return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}
