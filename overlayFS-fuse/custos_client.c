/* custos_client.c
 * 
 * custos_client interface implementation
 *
 * By Andy Sayler (www.andysayler.com)
 * Created  05/02/13
 *
 */

#include "custos_client.h"

#define RETURN_FAILURE -1
#define RETURN_SUCCESS  0

extern custosReq_t* custos_createReq(const custosUUID_t* uuid) {

    custosReq_t* req = NULL;


    if(!uuid) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createReq: 'uuid' must not be NULL\n");
#endif
	return NULL;
    }

    req = malloc(sizeof(*req));
    if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createReq: malloc failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }

#ifdef DEBUG
    fprintf(stderr, "INFO custos_createReq: uuid memcpy size = %z\n",
	    sizeof(req->uuid));
#endif
    memcpy(&(req->uuid), uuid, sizeof(req->uuid));
#ifdef DEBUG
    fprintf(stderr, "INFO custos_createReq: attrs memset size = %z\n",
	    (sizeof(custosAttr_t) * sizeof(req->attrs)));
#endif
    memset(req->attrs, 0, (sizeof(custosAttr_t) * sizeof(req->attrs)));

    return req;

}

extern int custos_getkey(const custosReq_t* req, custosRes_t* res) {

    (void) req;
    (void) res;

    return RETURN_FAILURE;
}
