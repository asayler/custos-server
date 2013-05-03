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

extern int custos_updateReq(custosReq_t* req, const custosAttrID_t id,
			    const void* value, const size_t size) {

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

    if(req->attrs[id].val) {
	free(req->attrs[id].val);
	req->attrs[id].val  = NULL;
	req->attrs[id].size = 0;
    }

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

    if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyReq: 'req' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyReq: '*req' must not be NULL\n");
#endif
	return -EINVAL;
    }

    for(i = 0; i < sizeof(req->attrs); i++) {
	if(req->attrs[i].val) {
	    free(req->attrs[i].val);
	    req->attrs[i].val  = NULL;
	    req->attrs[i].size = 0;
	}
    }

    free(req);
    req = NULL;

    return RETURN_SUCCESS;

}

extern int custos_getkey(const custosReq_t* req, custosRes_t* res) {

    (void) req;
    (void) res;

    return RETURN_FAILURE;
}
