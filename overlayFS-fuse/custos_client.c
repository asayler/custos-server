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
    attr = NULL;
    *attrp = NULL;

    return RETURN_SUCCESS;

}

extern custosAttrReq_t* custos_createAttrReq(const bool echo) {

    /* Local vars */
    custosAttrReq_t* attrreq = NULL;

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

    /* Populate */
    attrreq->echo = echo;

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

    /* Check and Free Optional Members */
    if(attrreq->attr) {
	if(custos_destroyAttr(&(attrreq->attr)) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_destroyAttrReq: custos_destroyAttr() failed\n");
#endif
	}
    }

    /* Free Struct */
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

extern int custos_updateKeyReqAddAttrReq(custosReq_t* req, custosAttrReq_t* attrreq) {

    /* Local vars */
    int index = 0;

   /* Input Invariant Check */
   if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateKeyReqAddAttrReq: 'req' must not be NULL\n");
#endif
	return -EINVAL;
   }
   if(!attrreq) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateKeyReqAddAttrReq: 'attrreq' must not be NULL\n");
#endif
	return -EINVAL;
   }

   /* Add Attr to Array */
   index = req->num_attrs;
   if(index >= CUS_MAX_ATTRS) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateKeyReqAddAttrReq: Max num_attrs exceeded\n");
#endif
	return -ERANGE;
   }
   req->attrs[index] = attrreq;
   req->num_attrs++;

   return index;

}

extern int custos_updateReqAddKeyReq(custosReq_t* req, custosKeyReq_t* keyreq) {

    /* Local vars */
    int index = 0;

   /* Input Invariant Check */
   if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateReqAddKeyReq: 'req' must not be NULL\n");
#endif
	return -EINVAL;
   }
   if(!keyreq) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateReqAddKeyReq: 'keyreq' must not be NULL\n");
#endif
	return -EINVAL;
   }

   /* Add Key to Array */
   index = req->num_keys;
   if(index >= CUS_MAX_KEYS) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateReqAddKeyReq: Max num_keys exceeded\n");
#endif
	return -ERANGE;
   }
   req->keys[index] = keyreq;
   req->num_keys++;

   return index;

}

extern custosRes_t* custos_getRes(const custosReq_t* req) {

   custosRes_t* res = NULL;

   if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_getRes: 'req' must not be NULL\n");
#endif
	return NULL;
   }

   if(!(req->target)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_getRes: 'req->target' must not be NULL\n");
#endif
	return NULL;
   }

   res = malloc(sizeof(*res));
   if(!res) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_getRes: malloc(res) failed\n");
	perror(         "------------------->");
#endif
	return NULL;
   }
   memset(res, 0, sizeof(*res));

   /* Populate */
   res->status = CUS_RESSTAT_ACCEPTED;

   /* ToDo: Make requet to custos server */

   /* Build Dummy Response */
   /* if(!(req->attrs[CUS_ATTRID_PSK].val)) { */
   /* 	res->attrStat[CUS_ATTRID_PSK] = CUS_ATTRSTAT_REQ; */
   /* } */
   /* else { */
   /* 	if(strcmp(req->attrs[CUS_ATTRID_PSK].val, CUS_TEST_PSK) == 0) { */
   /* 	    res->attrStat[CUS_ATTRID_PSK] = CUS_ATTRSTAT_GOOD; */
   /* 	    res->size = strlen(CUS_TEST_KEY) + 1; */
   /* 	    res->key = malloc(res->size); */
   /* 	    memcpy(res->key, CUS_TEST_KEY, res->size); */
   /* 	} */
   /* 	else { */
   /* 	    res->attrStat[CUS_ATTRID_PSK] = CUS_ATTRSTAT_BAD; */
   /* 	} */
   /* } */

   return res;
}

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
