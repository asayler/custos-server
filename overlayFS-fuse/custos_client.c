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

#define DEBUG

extern custosReq_t* custos_createReq(const uuid_t uuid, const char* uri) {

    custosReq_t* req = NULL;

    req = malloc(sizeof(*req));
    if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createReq: malloc(req) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }

    memset(req, 0, sizeof(*req));

    req->uri = strdup(uri);
    if(!(req->uri)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createReq: strdup(uri) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }

    uuid_copy(req->uuid, uuid);

    return req;

}

extern int custos_updateReq(custosReq_t* req, const custosAttrID_t id,
			    const void* value, const size_t size) {

    /* Input Invariant Check */
    if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateReq: 'req' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(id >= CUS_ATTRID_MAX) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateReq: 'id' must be less than %d\n",
		CUS_ATTRID_MAX);
#endif
	return -EINVAL;
    }
    if(!value) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateReq: 'value' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!size) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateReq: 'size' must not be 0\n");
#endif
	return -EINVAL;
    }

    /* Free Old Attribute if Set */
    if(req->attrs[id].val) {
	free(req->attrs[id].val);
	req->attrs[id].val  = NULL;
	req->attrs[id].size = 0;
    }

    /* Create and Set New Attribute */
    req->attrs[id].val = malloc(size);
    if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateReq: malloc failed\n");
	perror(         "---------------------->");
#endif
	return -errno;
    }
    req->attrs[id].size = size;
    memcpy(req->attrs[id].val, value, size);

    return RETURN_SUCCESS;

}

extern int custos_destroyReq(custosReq_t** reqp) {

    uint i;
    custosReq_t* req = *reqp;

    /* Input Invariant Check */
    if(!reqp) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyReq: 'reqp' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyReq: 'req' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Check and Free Required Memebers */
    if(!(req->uri)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyReq: 'req->uri' must not be NULL\n");
#endif	
    }
    free(req->uri);

    /* Check and Free Optional Memebers */
    for(i = 0; i < CUS_ATTRID_MAX; i++) {
	if(req->attrs[i].val) {
	    free(req->attrs[i].val);
	    req->attrs[i].val  = NULL;
	    req->attrs[i].size = 0;
	}
    }

    /* Free Struct */
    free(req);
    req = NULL;

    return RETURN_SUCCESS;

}

extern int custos_getkey(const custosReq_t* req, custosRes_t* res) {

    (void) req;
    (void) res;

    return RETURN_FAILURE;
}
