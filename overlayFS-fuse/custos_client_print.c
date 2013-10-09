/* custos_client_print.c
 *
 * custos_client_print implementation
 *
 * By Andy Sayler (www.andysayler.com)
 * Created  10/13
 *
 */

#include "custos_client_print.h"

#define DEBUG

#define RETURN_FAILURE -1
#define RETURN_SUCCESS  0

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

    fprintf(stream, "%*s" "attr->class   = %d\n",   offset, "", attr->class);
    fprintf(stream, "%*s" "attr->type    = %d\n",   offset, "", attr->type);
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
