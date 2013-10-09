/* custos_client_util.c
 *
 * custos_client utility tool
 *
 * By Andy Sayler (www.andysayler.com)
 * Created  05/02/13
 *
 */

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <uuid/uuid.h>

#include "custos_client.h"

#define DEBUG

#define RETURN_FAILURE -1
#define RETURN_SUCCESS  0

#define CUS_PRINT_OFFSET 4

char* custos_stringifyVal(size_t size, uint8_t* val) {

    char* out = NULL;

    if(!size) {
	out = strdup("");
	return out;
    }

    if(!val) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_printAttr: 'val' must not be NULL\n");
#endif
	errno = EINVAL;
	return NULL;
    }

    return out;

}

int custos_printAttr(custosAttr_t* attr, uint offset, FILE* stream) {

    if(!attr) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_printAttr: 'attr' must not be NULL\n");
#endif
	return -EINVAL;
    }

    fprintf(stream, "%*s" "attr->type    = %d\n",   offset, "", attr->type);
    fprintf(stream, "%*s" "attr->class   = %d\n",   offset, "", attr->class);
    fprintf(stream, "%*s" "attr->id      = %d\n",   offset, "", attr->id);
    fprintf(stream, "%*s" "attr->index   = %zd\n",  offset, "", attr->index);
    fprintf(stream, "%*s" "attr->size    = %zd\n",  offset, "", attr->size);
    fprintf(stream, "%*s" "attr->val     = %p\n",   offset, "", attr->val);

return RETURN_SUCCESS;

}

int custos_printAttrReq(custosAttrReq_t* attrreq, uint offset, FILE* stream) {

    int ret;

    if(!attrreq) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_printAttrReq: 'attrreq' must not be NULL\n");
#endif
	return -EINVAL;
    }

    fprintf(stream, "%*s" "attrreq->echo = %s\n", offset, "",
	    attrreq->echo ? "true" : "false");
    fprintf(stream, "%*s" "attrreq->attr = %p\n", offset, "", attrreq->attr);
    if(attrreq->attr) {
	ret = custos_printAttr(attrreq->attr, (offset + CUS_PRINT_OFFSET), stream);
	if(ret < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_printAttrReq: custos_printAttr() failed\n");
#endif
	    return ret;
	}
    }

    return RETURN_SUCCESS;

}

int custos_printAttrRes(custosAttrRes_t* attrres, uint offset, FILE* stream) {

    int ret;

    if(!attrres) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_printAttrRes: 'attrres' must not be NULL\n");
#endif
	return -EINVAL;
    }

    fprintf(stream, "%*s" "attrres->status = %d\n", offset, "", attrres->status);
    fprintf(stream, "%*s" "attrres->echo   = %s\n", offset, "",
	    attrres->echo ? "true" : "false");
    fprintf(stream, "%*s" "attrres->attr   = %p\n", offset, "", attrres->attr);
    if(attrres->attr) {
	ret = custos_printAttr(attrres->attr, (offset + CUS_PRINT_OFFSET), stream);
	if(ret < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_printAttrRes: custos_printAttr() failed\n");
#endif
	    return ret;
	}
    }

    return RETURN_SUCCESS;

}

int custos_printKey(custosKey_t* key, uint offset, FILE* stream) {

    char uuidstr[37];

    if(!key) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_printKey: 'key' must not be NULL\n");
#endif
	return -EINVAL;
    }

    uuid_unparse(key->uuid, uuidstr);

    fprintf(stream, "%*s" "key->uuid     = %s\n",          offset, "", uuidstr);
    fprintf(stream, "%*s" "key->revision = %" PRIu64 "\n", offset, "", key->revision);
    fprintf(stream, "%*s" "key->size     = %zd\n",         offset, "", key->size);
    fprintf(stream, "%*s" "key->val      = %p\n",          offset, "", key->val);

    return RETURN_SUCCESS;

}

int custos_printKeyReq(custosKeyReq_t* keyreq, uint offset, FILE* stream) {

    int ret;

    if(!keyreq) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_printKeyReq: 'key' must not be NULL\n");
#endif
	return -EINVAL;
    }

    fprintf(stream, "%*s" "keyreq->echo    = %s\n", offset, "",
	    keyreq->echo ? "true" : "false");
    fprintf(stream, "%*s" "keyreq->key     = %p\n", offset, "", keyreq->key);
    if(keyreq->key) {
	ret = custos_printKey(keyreq->key, (offset + CUS_PRINT_OFFSET), stream);
	if(ret < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_printKeyReq: custos_printKey() failed\n");
#endif
	    return ret;
	}
    }

    return RETURN_SUCCESS;

}

int custos_printKeyRes(custosKeyRes_t* keyres, uint offset, FILE* stream) {

    int ret;

    if(!keyres) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_printKeyRes: 'key' must not be NULL\n");
#endif
	return -EINVAL;
    }

    fprintf(stream, "%*s" "keyres->status  = %d\n", offset, "", keyres->status);
    fprintf(stream, "%*s" "keyres->echo    = %s\n", offset, "",
	    keyres->echo ? "true" : "false");
    fprintf(stream, "%*s" "keyres->key     = %p\n", offset, "", keyres->key);
    if(keyres->key) {
	ret = custos_printKey(keyres->key, (offset + CUS_PRINT_OFFSET), stream);
	if(ret < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_printKeyRes: custos_printKey() failed\n");
#endif
	    return ret;
	}
    }

    return RETURN_SUCCESS;

}

int custos_printReq(custosReq_t* req, uint offset, FILE* stream) {

    size_t i;
    uint newoffset = offset + CUS_PRINT_OFFSET;
    int ret;

    if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_printReq: 'req' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!req->target) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_printReq: 'req->target' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!req->version) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_printReq: 'req->version' must not be NULL\n");
#endif
	return -EINVAL;
    }

    fprintf(stream, "%*s" "req->target    = %s\n",  offset, "", req->target);
    fprintf(stream, "%*s" "req->version   = %s\n",  offset, "", req->version);
    fprintf(stream, "%*s" "req->num_attrs = %zd\n", offset, "", req->num_attrs);
    for(i = 0; i < req->num_attrs; i++) {
	fprintf(stream, "%*s" "req->attrs[%2zd] = %p\n",
		newoffset, "", i, req->attrs[i]);
	if(req->attrs[i]) {
	    ret = custos_printAttrReq(req->attrs[i], (newoffset + CUS_PRINT_OFFSET), stream);
	    if(ret < 0) {
#ifdef DEBUG
		fprintf(stderr, "ERROR custos_printReq: custos_printAttrReq() failed\n");
#endif
		return ret;
	    }
	}
    }
    fprintf(stream, "%*s" "req->num_keys  = %zd\n", offset, "", req->num_keys);
    for(i = 0; i < req->num_keys; i++) {
	fprintf(stream, "%*s" "req->keys[%2zd]  = %p\n",
		newoffset, "", i, req->keys[i]);
	if(req->keys[i]) {
	    ret = custos_printKeyReq(req->keys[i], (newoffset + CUS_PRINT_OFFSET), stream);
	    if(ret < 0) {
#ifdef DEBUG
		fprintf(stderr, "ERROR custos_printReq: custos_printKeyReq() failed\n");
#endif
		return ret;
	    }
	}
    }

    return RETURN_SUCCESS;

}

int custos_printRes(custosRes_t* res, uint offset, FILE* stream) {

    size_t i;
    uint newoffset = offset + CUS_PRINT_OFFSET;
    int ret;

    if(!res) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_printRes: 'res' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!res->source) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_printRes: 'res->source' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!res->version) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_printRes: 'res->version' must not be NULL\n");
#endif
	return -EINVAL;
    }

    fprintf(stream, "%*s" "res->status    = %d\n",  offset, "", res->status);
    fprintf(stream, "%*s" "res->source    = %s\n",  offset, "", res->source);
    fprintf(stream, "%*s" "res->version   = %s\n",  offset, "", res->version);
    fprintf(stream, "%*s" "res->num_attrs = %zd\n", offset, "", res->num_attrs);
    for(i = 0; i < res->num_attrs; i++) {
	fprintf(stream, "%*s" "res->attrs[%zd] = %p\n",
		newoffset, "", i, res->attrs[i]);
	if(res->attrs[i]) {
	    ret = custos_printAttrRes(res->attrs[i], (newoffset + CUS_PRINT_OFFSET), stream);
	    if(ret < 0) {
#ifdef DEBUG
		fprintf(stderr, "ERROR custos_printRes: custos_printAttrRes() failed\n");
#endif
		return ret;
	    }
	}
    }
    fprintf(stream, "%*s" "res->num_keys  = %zd\n", offset, "", res->num_keys);
    for(i = 0; i < res->num_keys; i++) {
	fprintf(stream, "%*s" "res->keys[%zd] = %p\n",
		newoffset, "", i, res->keys[i]);
	if(res->keys[i]) {
	    ret = custos_printKeyRes(res->keys[i], (newoffset + CUS_PRINT_OFFSET), stream);
	    if(ret < 0) {
#ifdef DEBUG
		fprintf(stderr, "ERROR custos_printRes: custos_printKeyRes() failed\n");
#endif
		return ret;
	    }
	}
    }

    return RETURN_SUCCESS;

}

int main(int argc, char* argv[]) {

    (void) argc;

    //int ret;
    uuid_t uuid;
    custosReq_t*    req    = NULL;
    custosKey_t*    key    = NULL;
    custosKeyReq_t* keyreq = NULL;
    custosRes_t*    res    = NULL;

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
	fprintf(stderr, "ERROR %s: custos_updateKeyReqAddKey() failed\n", argv[0]);
	return EXIT_FAILURE;
    }

    /* Print Request */
    if(custos_printReq(req, 0, stdout) < 0) {
	fprintf(stderr, "ERROR %s: custos_printReq() failed\n", argv[0]);
	return EXIT_FAILURE;
    }

    /* Get Key - 1st Attempt - Fails with */
    res = custos_getRes(req);
    if(!res) {
    	fprintf(stderr, "ERROR %s: custos_getRes() failed\n", argv[0]);
    	return EXIT_FAILURE;
    }

    /* Print Response */
    if(custos_printRes(res, 0, stdout) < 0) {
	fprintf(stderr, "ERROR %s: custos_printRes() failed\n", argv[0]);
	return EXIT_FAILURE;
    }

    /* /\* Free Response *\/ */
    /* ret = custos_destroyKeyRes(&res); */
    /* if(ret < 0) { */
    /* 	fprintf(stderr, "ERROR %s: custos_destroyKeyRes failed\n", argv[0]); */
    /* 	return EXIT_FAILURE; */
    /* } */
    /* if(res) { */
    /* 	fprintf(stderr, "ERROR %s: custos_destroyKeyRes failed to set res to NULL\n", argv[0]); */
    /* 	return EXIT_FAILURE; */
    /* } */

    /* /\* Update Request *\/ */
    /* ret = custos_updateKeyReq(req, CUS_ATTRID_PSK, CUS_TEST_BADPSK, (strlen(CUS_TEST_BADPSK) + 1)); */
    /* if(ret < 0) { */
    /* 	fprintf(stderr, "ERROR %s: custos_updateKeyReq failed\n", argv[0]); */
    /* 	return EXIT_FAILURE; */
    /* } */

    /* /\* Get Key - 2nd Attempt - Fails with CUSATTRSTAT_BAD on CUS_ATTRID_PSK *\/ */
    /* res = custos_getKeyRes(req); */
    /* if(!res) { */
    /* 	fprintf(stderr, "ERROR %s: custos_getKeyRes failed\n", argv[0]); */
    /* 	return EXIT_FAILURE; */
    /* } */

    /* /\* Print Response *\/ */
    /* fprintf(stdout, "res->resStat = %d\n", res->resStat); */
    /* if(res->key) { */
    /* 	fprintf(stdout, "res->key = %s\n", res->key); */
    /* } */
    /* else { */
    /* 	fprintf(stdout, "res->key = NULL\n"); */
    /* } */
    /* fprintf(stdout, "res->size = %zd\n", res->size); */
    /* fprintf(stdout, "res->attrStat[CUS_ATTRID_PSK] = %d\n", res->attrStat[CUS_ATTRID_PSK]); */

    /* /\* Free Response *\/ */
    /* ret = custos_destroyKeyRes(&res); */
    /* if(ret < 0) { */
    /* 	fprintf(stderr, "ERROR %s: custos_destroyKeyRes failed\n", argv[0]); */
    /* 	return EXIT_FAILURE; */
    /* } */
    /* if(res) { */
    /* 	fprintf(stderr, "ERROR %s: custos_destroyKeyRes failed to set res to NULL\n", argv[0]); */
    /* 	return EXIT_FAILURE; */
    /* } */

    /* /\* Update Request *\/ */
    /* ret = custos_updateKeyReq(req, CUS_ATTRID_PSK, CUS_TEST_PSK, (strlen(CUS_TEST_PSK) + 1)); */
    /* if(ret < 0) { */
    /* 	fprintf(stderr, "ERROR %s: custos_updateKeyReq failed\n", argv[0]); */
    /* 	return EXIT_FAILURE; */
    /* } */

    /* /\* Get Key - 3rd Attempt - Succeeds *\/ */
    /* res = custos_getKeyRes(req); */
    /* if(!res) { */
    /* 	fprintf(stderr, "ERROR %s: custos_getKeyRes failed\n", argv[0]); */
    /* 	return EXIT_FAILURE; */
    /* } */

    /* /\* Print Response *\/ */
    /* fprintf(stdout, "res->resStat = %d\n", res->resStat); */
    /* if(res->key) { */
    /* 	fprintf(stdout, "res->key = %s\n", res->key); */
    /* } */
    /* else { */
    /* 	fprintf(stdout, "res->key = NULL\n"); */
    /* } */
    /* fprintf(stdout, "res->size = %zd\n", res->size); */
    /* fprintf(stdout, "res->attrStat[CUS_ATTRID_PSK] = %d\n", res->attrStat[CUS_ATTRID_PSK]); */

    /* /\* Free Response *\/ */
    /* ret = custos_destroyKeyRes(&res); */
    /* if(ret < 0) { */
    /* 	fprintf(stderr, "ERROR %s: custos_destroyKeyRes failed\n", argv[0]); */
    /* 	return EXIT_FAILURE; */
    /* } */
    /* if(res) { */
    /* 	fprintf(stderr, "ERROR %s: custos_destroyKeyRes failed to set res to NULL\n", argv[0]); */
    /* 	return EXIT_FAILURE; */
    /* }     */

    /* /\* Free Request *\/ */
    /* ret = custos_destroyKeyReq(&req); */
    /* if(ret < 0) { */
    /* 	fprintf(stderr, "ERROR %s: custos_destroyKeyReq failed\n", argv[0]); */
    /* 	return EXIT_FAILURE; */
    /* } */
    /* if(req) { */
    /* 	fprintf(stderr, "ERROR %s: custos_destroyKeyReq failed to set req to NULL\n", argv[0]); */
    /* 	return EXIT_FAILURE; */
    /* } */

    /* Clean up */
    if(custos_destroyReq(&req) < 0) {
	fprintf(stderr, "ERROR %s: custos_destroyReq() failed\n", argv[0]);
	return EXIT_FAILURE;
    }
    if(custos_destroyRes(&res) < 0) {
	fprintf(stderr, "ERROR %s: custos_destroyRes() failed\n", argv[0]);
	return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

}
