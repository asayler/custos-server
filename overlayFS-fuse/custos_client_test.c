/* custos_client_test.c
 *
 * custos_client unit tests
 *
 * By Andy Sayler (www.andysayler.com)
 * Created  05/02/13
 *
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <uuid/uuid.h>

#include "custos_client.h"
#include "custos_client_print.h"

#define BAD_PSK "Nonsense"

int main(int argc, char* argv[]) {

    (void) argc;

    uuid_t uuid;
    custosReq_t*     req     = NULL;
    custosKey_t*     key     = NULL;
    custosKeyReq_t*  keyreq  = NULL;
    custosAttr_t*    attr    = NULL;
    custosAttrReq_t* attrreq = NULL;
    custosRes_t*     res     = NULL;
    json_object*     reqjson = NULL;
    const char*      reqstr  = NULL;

    /* Setup a new request */
    req = custos_createReq("http://test.com");
    if(!req) {
	fprintf(stderr, "ERROR %s: custos_createKeyReq() failed\n", argv[0]);
	return EXIT_FAILURE;
    }

    /* Add Key to Request */
    uuid_generate(uuid);
    key = custos_createKey(uuid, 1, 0, NULL);
    if(!key) {
	fprintf(stderr, "ERROR %s: custos_createKey() failed\n", argv[0]);
	return EXIT_FAILURE;
    }
    keyreq = custos_createKeyReq(true);
    if(!keyreq) {
	fprintf(stderr, "ERROR %s: custos_createKeyReq() failed\n", argv[0]);
	return EXIT_FAILURE;
    }
    if(custos_updateKeyReqAddKey(keyreq, key) < 0) {
	fprintf(stderr, "ERROR %s: custos_updateKeyReqAddKey() failed\n", argv[0]);
	return EXIT_FAILURE;
    }
    if(custos_updateReqAddKeyReq(req, keyreq) < 0) {
	fprintf(stderr, "ERROR %s: custos_updateReqAddKeyReq() failed\n", argv[0]);
	return EXIT_FAILURE;
    }

    /* Print Request */
    fprintf(stdout, "********* First  Request *********\n");
    if(custos_printReq(req, 0, stdout) < 0) {
	fprintf(stderr, "ERROR %s: custos_printReq() failed\n", argv[0]);
	return EXIT_FAILURE;
    }
    fprintf(stdout, "\n");

    /* Print Request Json */
    fprintf(stdout, "******* First  Request Json *******\n");
    reqjson = custos_reqToJson(req);
    if(!reqjson) {
	fprintf(stderr, "ERROR %s: custos_ReqToJson() failed\n", argv[0]);
	return EXIT_FAILURE;
    }
    reqstr = json_object_to_json_string(reqjson);
    if(!reqstr) {
	fprintf(stderr, "ERROR %s: json_object_to_json_string() failed\n", argv[0]);
	return EXIT_FAILURE;
    }
    fprintf(stdout, "%s\n", reqstr);
    fprintf(stdout, "\n");
    json_object_put(reqjson);
    reqjson = NULL;

    /* Get Response - 1st Attempt - Fails */
    res = custos_getRes(req);
    if(!res) {
    	fprintf(stderr, "ERROR %s: custos_getRes() failed\n", argv[0]);
    	return EXIT_FAILURE;
    }

    /* Print Response */
    fprintf(stdout, "********* First Response ********\n");
    if(custos_printRes(res, 0, stdout) < 0) {
	fprintf(stderr, "ERROR %s: custos_printRes() failed\n", argv[0]);
	return EXIT_FAILURE;
    }
    fprintf(stdout, "\n");

    /* Free Response */
    if(custos_destroyRes(&res) < 0) {
	fprintf(stderr, "ERROR %s: custos_destroyRes() failed\n", argv[0]);
	return EXIT_FAILURE;
    }

    /* Add attr to request */
    attr = custos_createAttr(CUS_ATTRCLASS_EXPLICIT, CUS_ATTRTYPE_EXP_PSK, 0,
			     (strlen(BAD_PSK) + 1), (uint8_t*) BAD_PSK);
    if(!attr) {
	fprintf(stderr, "ERROR %s: custos_createAttr() failed\n", argv[0]);
	return EXIT_FAILURE;
    }
    attrreq = custos_createAttrReq(true);
    if(!attrreq) {
	fprintf(stderr, "ERROR %s: custos_createAttrReq() failed\n", argv[0]);
	return EXIT_FAILURE;
    }
    if(custos_updateAttrReqAddAttr(attrreq, attr) < 0) {
	fprintf(stderr, "ERROR %s: custos_updateAttrReqAddAttr() failed\n", argv[0]);
	return EXIT_FAILURE;
    }
    if(custos_updateReqAddAttrReq(req, attrreq) < 0) {
	fprintf(stderr, "ERROR %s: custos_updateReqAddAttrReq() failed\n", argv[0]);
	return EXIT_FAILURE;
    }

    /* Print Request */
    fprintf(stdout, "********* Second  Request *********\n");
    if(custos_printReq(req, 0, stdout) < 0) {
	fprintf(stderr, "ERROR %s: custos_printReq() failed\n", argv[0]);
	return EXIT_FAILURE;
    }
    fprintf(stdout, "\n");

    /* Get Response - 2nd Attempt - Fails */
    res = custos_getRes(req);
    if(!res) {
    	fprintf(stderr, "ERROR %s: custos_getRes() failed\n", argv[0]);
    	return EXIT_FAILURE;
    }

    /* Print Response */
    fprintf(stdout, "********* Second Response ********\n");
    if(custos_printRes(res, 0, stdout) < 0) {
	fprintf(stderr, "ERROR %s: custos_printRes() failed\n", argv[0]);
	return EXIT_FAILURE;
    }
    fprintf(stdout, "\n");

    /* Free Response */
    if(custos_destroyRes(&res) < 0) {
	fprintf(stderr, "ERROR %s: custos_destroyRes() failed\n", argv[0]);
	return EXIT_FAILURE;
    }

    /* Update attr in request */
    if(custos_updateAttr(attr,
			 CUS_ATTRCLASS_EXPLICIT, CUS_ATTRTYPE_EXP_PSK, 0,
			 (strlen(CUS_TEST_PSK_GOOD) + 1), (uint8_t*) CUS_TEST_PSK_GOOD) < 0) {
	fprintf(stderr, "ERROR %s: custos_updateAttr() failed\n", argv[0]);
	return EXIT_FAILURE;
    }

    /* Print Request */
    fprintf(stdout, "********* Third  Request *********\n");
    if(custos_printReq(req, 0, stdout) < 0) {
	fprintf(stderr, "ERROR %s: custos_printReq() failed\n", argv[0]);
	return EXIT_FAILURE;
    }
    fprintf(stdout, "\n");

   /* Get Response - 3rd Attempt - Succeeds */
    res = custos_getRes(req);
    if(!res) {
    	fprintf(stderr, "ERROR %s: custos_getRes() failed\n", argv[0]);
    	return EXIT_FAILURE;
    }

    /* Print Response */
    fprintf(stdout, "********* Third Response ********\n");
    if(custos_printRes(res, 0, stdout) < 0) {
	fprintf(stderr, "ERROR %s: custos_printRes() failed\n", argv[0]);
	return EXIT_FAILURE;
    }
    fprintf(stdout, "\n");

   /* Free Response */
    if(custos_destroyRes(&res) < 0) {
	fprintf(stderr, "ERROR %s: custos_destroyRes() failed\n", argv[0]);
	return EXIT_FAILURE;
    }

    /* Free Request */
    if(custos_destroyReq(&req) < 0) {
	fprintf(stderr, "ERROR %s: custos_destroyReq() failed\n", argv[0]);
	return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}
