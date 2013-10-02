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

extern custosAttrReq_t* custos_createAttrReq(const custosAttrType_t type,
					     const custosAttrClass_t class,
					     const custosAttrID_t id,
					     const size_t index,
					     const size_t size, const uint8_t* val,
					     const bool echo) {

    /* Local vars */
    custosAttrReq_t* attrreq = NULL;

    /* Input Invariant Check */
    if(type >= CUS_ATTRTYPE_MAX) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createAttrReq: 'type' must not be less than %d\n",
		CUS_ATTRTYPE_MAX);
#endif
	errno = EINVAL;
	return NULL;
    }
    if(class >= CUS_ATTRCLASS_MAX) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createAttrReq: 'class' must not be less than %d\n",
		CUS_ATTRCLASS_MAX);
#endif
	errno = EINVAL;
	return NULL;
    }
    if(id >= CUS_ATTRID_MAX) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createAttrReq: 'id' must be less than %d\n",
		CUS_ATTRID_MAX);
#endif
	errno = EINVAL;
	return NULL;
    }

    /* Initialize Vars */
    attrreq = malloc(sizeof(*attrreq));
    if(!attrreq) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createAttrReq: malloc(attrreq) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }
    memset(attrreq, 0, sizeof(*attrreq));

    attrreq->attr = malloc(sizeof(*(attrreq->attr)));
    if(!(attrreq->attr)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createAttrReq: malloc(attrreq->attr) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }
    memset(attrreq->attr, 0, sizeof(*(attrreq->attr)));

    /* Populate */
    attrreq->attr->type = type;
    attrreq->attr->class = class;
    attrreq->attr->id = id;
    attrreq->attr->index = index;
    attrreq->attr->size = size;
    attrreq->echo = echo;

    if((attrreq->attr->size) > 0) {
	/* Validate val */
	if(!val) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_createAttrReq: 'val' can't be null when size non-zero\n");
#endif
	    errno = EINVAL;
	    return NULL;
	}
	/* Create and Set New Attribute */
	attrreq->attr->val = malloc(attrreq->attr->size);
	if(!(attrreq->attr->val)) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_createAttrReq: malloc(attrreq->attr->size) failed\n");
	    perror(         "---------------------->");
#endif
	    return NULL;
	}
	memcpy(attrreq->attr->val, val, attrreq->attr->size);
    }

    return attrreq;

}

extern int custos_destroyAttrReq(custosAttrReq_t** attrreqp) {

    /* Local Vars */
    custosAttrReq_t* attrreq;

    /* Input Invariant Check */
    if(!attrreqp) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyAttrReq: 'attrreqp' must not be NULL\n");
#endif
	return -EINVAL;
    }
    attrreq = *attrreqp;

    if(!attrreq) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyAttrReq: 'attrreq' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Check and Free Required Members */
    if(!(attrreq->attr)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyAttrReq: 'attrreq->attr' must not be NULL\n");
#endif
	return -EINVAL;
    }
    free(attrreq->attr);
    attrreq->attr = NULL;

    /* Check and Free Optional Members */
    if(attrreq->attr->val) {
	free(attrreq->attr->val);
	attrreq->attr->val = NULL;
    }

    /* Free Outer Struct */
    free(attrreq);
    attrreq = NULL;
    *attrreqp = NULL;

    return RETURN_SUCCESS;

}

extern custosKeyReq_t* custos_createKeyReq(const uuid_t uuid,
					   const uint64_t version,
					   const size_t size, const uint8_t* val,
					   const bool echo) {

    /* Local vars */
    custosKeyReq_t* keyreq = NULL;

    /* Input Invariant Check */
    if(uuid_is_null(uuid)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createKeyReq: 'uuid' must not be null\n");
#endif
	errno = EINVAL;
	return NULL;
    }

    /* Initialize Vars */
    keyreq = malloc(sizeof(*keyreq));
    if(!keyreq) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createKeyReq: malloc(keyreq) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }
    memset(keyreq, 0, sizeof(*keyreq));

    keyreq->key = malloc(sizeof(*(keyreq->key)));
    if(!(keyreq->key)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createKeyReq: malloc(keyreq->key) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }
    memset(keyreq->key, 0, sizeof(*(keyreq->key)));

    /* Populate */
    uuid_copy(keyreq->key->uuid, uuid);
    keyreq->key->version = version;
    keyreq->key->size = size;
    keyreq->echo = echo;

    if((keyreq->key->size) > 0) {
	/* Validate val */
	if(!val) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_createKeyReq: 'val' can't be null when size non-zero\n");
#endif
	    errno = EINVAL;
	    return NULL;
	}
	/* Create and Set New Attribute */
	keyreq->key->val = malloc(keyreq->key->size);
	if(!(keyreq->key->val)) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_createKeyReq: malloc(keyreq->key->size) failed\n");
	    perror(         "---------------------->");
#endif
	    return NULL;
	}
	memcpy(keyreq->key->val, val, keyreq->key->size);
    }

    return keyreq;

}

extern int custos_destroyKeyReq(custosKeyReq_t** keyreqp) {

    /* Local Vars */
    custosKeyReq_t* keyreq;

    /* Input Invariant Check */
    if(!keyreqp) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyKeyReq: 'keyreqp' must not be NULL\n");
#endif
	return -EINVAL;
    }
    keyreq = *keyreqp;

    if(!keyreq) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyKeyReq: 'keyreq' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Check and Free Required Members */
    if(!(keyreq->key)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyKeyReq: 'keyreq->key' must not be NULL\n");
#endif
	return -EINVAL;
    }
    free(keyreq->key);
    keyreq->key = NULL;

    /* Check and Free Optional Members */
    if(keyreq->key->val) {
	free(keyreq->key->val);
	keyreq->key->val = NULL;
    }

    /* Free Outer Struct */
    free(keyreq);
    keyreq = NULL;
    *keyreqp = NULL;

    return RETURN_SUCCESS;

}

extern custosReq_t* custos_createReq(const char* target) {

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

   req->target = strdup(target);
   if(!(req->target)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createReq: strdup(target) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
   }

   req->version = strdup(CUS_VERSION);
   if(!(req->version)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createReq: strdup(CUSTOS_VERSION) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
   }
   req->num_attrs = 0;
   req->num_keys = 0;

   return req;

}

extern int custos_destroyReq(custosReq_t** reqp) {

   /* Local vars */
   uint i;
   custosReq_t* req;

   /* Input Invariant Check */
   if(!reqp) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyReq: 'reqp' must not be NULL\n");
#endif
	return -EINVAL;
   }
   req = *reqp;

   if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyReq: 'req' must not be NULL\n");
#endif
	return -EINVAL;
   }

   /* Check and Free Required Members */
   if(!(req->target)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyReq: 'req->target' must not be NULL\n");
#endif
   }
   free(req->target);

   if(!(req->version)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyReq: 'req->version' must not be NULL\n");
#endif
   }
   free(req->version);

   /* Check and Free Optional Members */
   /* Free Attrs */
   for(i = 0; i < req->num_attrs; i++) {
       if(custos_destroyAttrReq(&(req->attrs[i])) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_destroyReq: custos_destroyAttrReq() failed\n");
#endif
	}
   }
   req->num_attrs = 0;

   /* Free Keys */
   for(i = 0; i < req->num_keys; i++) {
       if(custos_destroyKeyReq(&(req->keys[i])) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_destroyReq: custos_destroyKeyReq() failed\n");
#endif
	}
   }
   req->num_keys = 0;

   /* Free Struct */
   free(req);
   req = NULL;
   *reqp = NULL;

   return RETURN_SUCCESS;

}

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
