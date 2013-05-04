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
    custosUUID_t uuid;
    custosReq_t* req;
    char* pw = "password";

    req = custos_createReq(&uuid, "http://test.com");
    if(!req) {
	fprintf(stderr, "ERROR main: custos_createReq failed\n");
	return EXIT_FAILURE;
    }
    
    ret = custos_updateReq(req, CUS_ATTRID_PSK, pw, 9);
    if(ret < 0) {
    	fprintf(stderr, "ERROR main: custos_updateReq failed\n");
    	return EXIT_FAILURE;
    }
    ret = custos_updateReq(req, CUS_ATTRID_PSK, pw, 9);
    if(ret < 0) {
    	fprintf(stderr, "ERROR main: custos_updateReq failed\n");
    	return EXIT_FAILURE;
    }

    ret = custos_destroyReq(&req);
    if(ret < 0) {
	fprintf(stderr, "ERROR main: custos_destroyReq failed\n");
	return EXIT_FAILURE;
    }
    if(req) {
	fprintf(stderr, "ERROR main: custos_destroyReq failed to set req to NULL\n");
	return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}
