/* custos_client.c
 * 
 * custos_client interface implementation
 *
 * By Andy Sayler (www.andysayler.com)
 * Created  05/02/13
 *
 */

#include "custos_client.h"

#define DEBUG

#define RETURN_FAILURE -1
#define RETURN_SUCCESS  0

extern custosKeyReq_t* custos_createKeyReq(const uuid_t uuid, const char* uri) {

    custosKeyReq_t* req = NULL;

    req = malloc(sizeof(*req));
    if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createKeyReq: malloc(req) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }
    memset(req, 0, sizeof(*req));

    req->uri = strdup(uri);
    if(!(req->uri)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createKeyReq: strdup(uri) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }

    uuid_copy(req->uuid, uuid);

    return req;

}

extern int custos_updateKeyReq(custosKeyReq_t* req, const custosAttrID_t id,
			    const void* value, const size_t size) {

    /* Input Invariant Check */
    if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateKeyReq: 'req' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(id >= CUS_ATTRID_MAX) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateKeyReq: 'id' must be less than %d\n",
		CUS_ATTRID_MAX);
#endif
	return -EINVAL;
    }
    if(!value) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateKeyReq: 'value' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!size) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateKeyReq: 'size' must not be 0\n");
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
	fprintf(stderr, "ERROR custos_updateKeyReq: malloc failed\n");
	perror(         "---------------------->");
#endif
	return -errno;
    }
    req->attrs[id].size = size;
    memcpy(req->attrs[id].val, value, size);

    return RETURN_SUCCESS;

}

extern int custos_destroyKeyReq(custosKeyReq_t** reqp) {

    uint i;
    custosKeyReq_t* req = *reqp;

    /* Input Invariant Check */
    if(!reqp) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyKeyReq: 'reqp' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyKeyReq: 'req' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Check and Free Required Members */
    if(!(req->uri)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyKeyReq: 'req->uri' must not be NULL\n");
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
    *reqp = NULL;

    return RETURN_SUCCESS;

}

extern custosKeyRes_t* custos_getKey(const custosKeyReq_t* req) {
    
    custosKeyRes_t* res = NULL;
    
    if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_getKey: 'req' must not be NULL\n");
#endif
	return NULL;
    }
    
    if(!(req->uri)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_getKey: 'req->uri' must not be NULL\n");
#endif
	return NULL;
    }
    
    res = malloc(sizeof(*res));
    if(!res) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_getReq: malloc(res) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }
    memset(res, 0, sizeof(*res));
    res->key = NULL;
    res->size = 0;

    /* ToDo: Make requet to custos server */
    
    /* Build Dummy Response */
    if(!(req->attrs[CUS_ATTRID_PSK].val)) {
	res->attrStat[CUS_ATTRID_PSK] = CUS_ATTRSTAT_REQ;
    }
    else {
	if(strcmp(req->attrs[CUS_ATTRID_PSK].val, TEST_PSK) == 0) {
	    res->attrStat[CUS_ATTRID_PSK] = CUS_ATTRSTAT_GOOD;
	    res->size = strlen(TEST_KEY) + 1;
	    res->key = malloc(res->size);
	    memcpy(res->key, TEST_KEY, res->size);
	}
	else {
	    res->attrStat[CUS_ATTRID_PSK] = CUS_ATTRSTAT_BAD;
	}
    }

    return res;
}

extern int custos_destroyKeyRes(custosKeyRes_t** resp) {

    (void) resp;

    return RETURN_FAILURE;
}
