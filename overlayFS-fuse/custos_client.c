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

#define CUS_TEST_KEY "myfakekey"

#define RETURN_FAILURE -1
#define RETURN_SUCCESS  0


extern custosAttr_t* custos_createAttr(const custosAttrType_t type,
				       const custosAttrClass_t class,
				       const custosAttrID_t id,
				       const size_t index,
				       const size_t size, const uint8_t* val) {

    /* Local vars */
    custosAttr_t* attr = NULL;

    /* Input Invariant Check */
    if(type >= CUS_ATTRTYPE_MAX) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createAttr: 'type' must not be less than %d\n",
		CUS_ATTRTYPE_MAX);
#endif
	errno = EINVAL;
	return NULL;
    }
    if(class >= CUS_ATTRCLASS_MAX) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createAttr: 'class' must not be less than %d\n",
		CUS_ATTRCLASS_MAX);
#endif
	errno = EINVAL;
	return NULL;
    }
    if(id >= CUS_ATTRID_MAX) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createAttr: 'id' must be less than %d\n",
		CUS_ATTRID_MAX);
#endif
	errno = EINVAL;
	return NULL;
    }

    /* Initialize Vars */
    attr = malloc(sizeof(*attr));
    if(!attr) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createAttr: malloc(attr) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }
    memset(attr, 0, sizeof(*attr));

    /* Populate */
    attr->type = type;
    attr->class = class;
    attr->id = id;
    attr->index = index;
    attr->size = size;
    if((attr->size) > 0) {
	/* Validate val */
	if(!val) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_createAttr: 'val' can't be null when size non-zero\n");
#endif
	    errno = EINVAL;
	    return NULL;
	}
	/* Create and Set New Attribute */
	attr->val = malloc(attr->size);
	if(!(attr->val)) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_createAttr: malloc(attr->size) failed\n");
	    perror(         "---------------------->");
#endif
	    return NULL;
	}
	memcpy(attr->val, val, attr->size);
    }

    return attr;

}

extern int custos_destroyAttr(custosAttr_t** attrp) {

    /* Local Vars */
    custosAttr_t* attr;

    /* Input Invariant Check */
    if(!attrp) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyAttr: 'attrp' must not be NULL\n");
#endif
	return -EINVAL;
    }
    attr = *attrp;

    if(!attr) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyAttr: 'attr' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Check and Free Optional Members */
    if(attr->val) {
	free(attr->val);
	attr->val = NULL;
    }

    /* Free Struct */
    free(attr);
    *attrp = NULL;

    return RETURN_SUCCESS;

}

extern custosKey_t* custos_createKey(const uuid_t uuid,
				     const uint64_t version,
				     const size_t size, const uint8_t* val) {

    /* Local vars */
    custosKey_t* key = NULL;

    /* Input Invariant Check */
    if(uuid_is_null(uuid)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createKey: 'uuid' must not be null\n");
#endif
	errno = EINVAL;
	return NULL;
    }

    /* Initialize Vars */
    key = malloc(sizeof(*key));
    if(!key) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createKey: malloc(*key) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }
    memset(key, 0, sizeof(*key));

    /* Populate */
    uuid_copy(key->uuid, uuid);
    key->version = version;
    key->size = size;
    if((key->size) > 0) {
	/* Validate val */
	if(!val) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_createKey: 'val' can't be null when size non-zero\n");
#endif
	    errno = EINVAL;
	    return NULL;
	}
	/* Create and Set New Attribute */
	key->val = malloc(key->size);
	if(!(key->val)) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_createKey: malloc(key->size) failed\n");
	    perror(         "---------------------->");
#endif
	    return NULL;
	}
	memcpy(key->val, val, key->size);
    }

    return key;

}

extern int custos_destroyKey(custosKey_t** keyp) {

    /* Local Vars */
    custosKey_t* key;

    /* Input Invariant Check */
    if(!keyp) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyKey: 'keyp' must not be NULL\n");
#endif
	return -EINVAL;
    }
    key = *keyp;

    if(!key) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyKey: 'key' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Check and Free Optional Members */
    if(key->val) {
	free(key->val);
	key->val = NULL;
    }

    /* Free Struct */
    free(key);
    *keyp = NULL;

    return RETURN_SUCCESS;

}

/* extern custosReq_t* custos_createReq(const char* target) { */

/*    custosReq_t* req = NULL; */

/*    req = malloc(sizeof(*req)); */
/*    if(!req) { */
/* #ifdef DEBUG */
/* 	fprintf(stderr, "ERROR custos_createReq: malloc(req) failed\n"); */
/* 	perror(         "---------------------->"); */
/* #endif */
/* 	return NULL; */
/*    } */
/*    memset(req, 0, sizeof(*req)); */

/*    req->target = strdup(target); */
/*    if(!(req->target)) { */
/* #ifdef DEBUG */
/* 	fprintf(stderr, "ERROR custos_createReq: strdup(target) failed\n"); */
/* 	perror(         "---------------------->"); */
/* #endif */
/* 	return NULL; */
/*    } */

/*    return req; */

/* } */

/* extern int custos_updateReqAddKey(custosReq_t* req, const CustosKeyReq_t* key) { */

/*    /\* Input Invariant Check *\/ */
/*    if(!req) { */
/* #ifdef DEBUG */
/* 	fprintf(stderr, "ERROR custos_updateReqAddKey: 'req' must not be NULL\n"); */
/* #endif */
/* 	return -EINVAL; */
/*    } */
/*    if(!key) { */
/* #ifdef DEBUG */
/* 	fprintf(stderr, "ERROR custos_updateReqAddKey: 'key' must not be NULL\n"); */
/* #endif */
/* 	return -EINVAL; */
/*    } */

/*    /\* Add Key to Array *\/ */
/*    /\* TODO Add handling for re-adding keys *\/ */
/*    if(req->num_keys >= CUS_MAX_KEYS) { */
/* #ifdef DEBUG */
/* 	fprintf(stderr, "ERROR custos_updateReqAddKey: Max num_keys exceeded\n"); */
/* #endif */
/* 	return -ERANGE */
/*    } */
/*    req->keys[req->num_keys] = key; */
/*    req->num_keys++; */

/*    return RETURN_SUCCESS; */

/* } */

/* extern int custos_updateKeyReqAddAttr(custosKeyReq_t* req, const CustosAttr_t* attr) { */

/*    /\* Input Invariant Check *\/ */
/*    if(!req) { */
/* #ifdef DEBUG */
/* 	fprintf(stderr, "ERROR custos_updateKeyReqAddAttr: 'req' must not be NULL\n"); */
/* #endif */
/* 	return -EINVAL; */
/*    } */
/*    if(!attr) { */
/* #ifdef DEBUG */
/* 	fprintf(stderr, "ERROR custos_updateKeyReqAddAttr: 'attr' must not be NULL\n"); */
/* #endif */
/* 	return -EINVAL; */
/*    } */

/*    /\* Add Attr to Array *\/ */
/*    /\* TODO Add handling for re-adding attrs *\/ */
/*    if(req->num_attrs >= CUS_MAX_ATTRS) { */
/* #ifdef DEBUG */
/* 	fprintf(stderr, "ERROR custos_updateKeyReqAddAttr: Max num_attrs exceeded\n"); */
/* #endif */
/* 	return -ERANGE */
/*    } */
/*    req->attrs[req->num_attrs] = attr; */
/*    req->num_attrs++; */

/*    return RETURN_SUCCESS; */

/* } */

/* extern int custos_destroyKeyReq(custosKeyReq_t** reqp) { */

/*    /\* Local vars *\/ */
/*    uint i; */
/*    custosKeyReq_t* req; */

/*    /\* Input Invariant Check *\/ */
/*    if(!reqp) { */
/* #ifdef DEBUG */
/* 	fprintf(stderr, "ERROR custos_destroyKeyReq: 'reqp' must not be NULL\n"); */
/* #endif */
/* 	return -EINVAL; */
/*    } */
/*    req = *reqp; */

/*    if(!req) { */
/* #ifdef DEBUG */
/* 	fprintf(stderr, "ERROR custos_destroyKeyReq: 'req' must not be NULL\n"); */
/* #endif */
/* 	return -EINVAL; */
/*    } */

/*    /\* Check and Free Required Members *\/ */
/*    if(!(req->uri)) { */
/* #ifdef DEBUG */
/* 	fprintf(stderr, "ERROR custos_destroyKeyReq: 'req->uri' must not be NULL\n"); */
/* #endif */
/*    } */
/*    free(req->uri); */

/*    /\* Check and Free Optional Members *\/ */
/*    /\* Free Attrs *\/ */
/*    for(i = 0; i < req->num_attrs; i++) { */
/* 	if(custos_destroyAttr(&attrs[i]) < 0){ */
/* #ifdef DEBUG */
/* 	    fprintf(stderr, "ERROR custos_destroyKeyReq: custos_destroyAttr() failed\n"); */
/* #endif */
/* 	} */
/*    } */
/*    req->num_attrs = 0; */
/*    /\* Free Keys *\/ */
/*    for(i = 0; i < req->num_keys; i++) { */
/* 	if(custos_destroyKey(&keys[i]) < 0){ */
/* #ifdef DEBUG */
/* 	    fprintf(stderr, "ERROR custos_destroyKeyReq: custos_destroyKey() failed\n"); */
/* #endif */
/* 	} */
/*    } */
/*    req->num_keys = 0;     */

/*    /\* Free Struct *\/ */
/*    free(req); */
/*    *reqp = NULL; */

/*    return RETURN_SUCCESS; */

/* } */

/* extern custosKeyRes_t* custos_getKeyRes(const custosKeyReq_t* req) { */

/*    custosKeyRes_t* res = NULL; */

/*    if(!req) { */
/* #ifdef DEBUG */
/* 	fprintf(stderr, "ERROR custos_getKey: 'req' must not be NULL\n"); */
/* #endif */
/* 	return NULL; */
/*    } */

/*    if(!(req->uri)) { */
/* #ifdef DEBUG */
/* 	fprintf(stderr, "ERROR custos_getKey: 'req->uri' must not be NULL\n"); */
/* #endif */
/* 	return NULL; */
/*    } */

/*    res = malloc(sizeof(*res)); */
/*    if(!res) { */
/* #ifdef DEBUG */
/* 	fprintf(stderr, "ERROR custos_getReq: malloc(res) failed\n"); */
/* 	perror(         "---------------------->"); */
/* #endif */
/* 	return NULL; */
/*    } */
/*    memset(res, 0, sizeof(*res)); */
/*    res->resStat = CUS_RESSTAT_GOOD; */
/*    res->key = NULL; */
/*    res->size = 0; */

/*    /\* ToDo: Make requet to custos server *\/ */

/*    /\* Build Dummy Response *\/ */
/*    if(!(req->attrs[CUS_ATTRID_PSK].val)) { */
/* 	res->attrStat[CUS_ATTRID_PSK] = CUS_ATTRSTAT_REQ; */
/*    } */
/*    else { */
/* 	if(strcmp(req->attrs[CUS_ATTRID_PSK].val, CUS_TEST_PSK) == 0) { */
/* 	    res->attrStat[CUS_ATTRID_PSK] = CUS_ATTRSTAT_GOOD; */
/* 	    res->size = strlen(CUS_TEST_KEY) + 1; */
/* 	    res->key = malloc(res->size); */
/* 	    memcpy(res->key, CUS_TEST_KEY, res->size); */
/* 	} */
/* 	else { */
/* 	    res->attrStat[CUS_ATTRID_PSK] = CUS_ATTRSTAT_BAD; */
/* 	} */
/*    } */

/*    return res; */
/* } */

/* extern int custos_destroyKeyRes(custosKeyRes_t** resp) { */

/*    custosKeyRes_t* res; */

/*    /\* Input Invariant Check *\/ */
/*    if(!resp) { */
/* #ifdef DEBUG */
/* 	fprintf(stderr, "ERROR custos_destroyKeyRes: 'resp' must not be NULL\n"); */
/* #endif */
/* 	return -EINVAL; */
/*    } */
/*    res = *resp; */

/*    if(!res) { */
/* #ifdef DEBUG */
/* 	fprintf(stderr, "ERROR custos_destroyKeyRes: 'res' must not be NULL\n"); */
/* #endif */
/* 	return -EINVAL; */
/*    } */

/*    /\* Check and Free Optional Members *\/ */
/*    if(res->key) { */
/* 	free(res->key); */
/*    } */

/*    /\* Free Struct *\/ */
/*    free(res); */
/*    *resp = NULL; */

/*    return RETURN_SUCCESS; */

/* } */
