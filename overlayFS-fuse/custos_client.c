/* custos_client.c
 *
 * custos_client interface implementation
 *
 * By Andy Sayler (www.andysayler.com)
 * Created  05/02/13
 *
 */

#include "custos_client.h"
#include "custos_client_print.h"

#define DEBUG

#define TEST_KEY "This is my fake pre-shared key!"

#define RETURN_SUCCESS  0

/********* JSON Functions *********/

extern json_object* custos_AttrToJson(const custosAttr_t* attr) {

    json_object* json = NULL;
    const char* classStr = NULL;
    const char* typeStr = NULL;
    char*  b64Data = NULL;
    size_t b64Size = 0;

    /* Validate Input */
    if(!attr) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_AttrToJson: 'attr' must not be NULL\n");
#endif
	return NULL;
    }

    /* Process Top Level Object */
    json = json_object_new_object();
    if(!json) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_AttrToJson: json_object_new_object() failed\n");
#endif
	return NULL;
    }
    classStr = custos_AttrClassToStr(attr->class);
    json_object_object_add(json, "Class", json_object_new_string(classStr));
    typeStr = custos_AttrTypeToStr(attr->class, attr->type);
    json_object_object_add(json, "Type",  json_object_new_string(typeStr));
    json_object_object_add(json, "Index", json_object_new_int64(attr->index));

    /* Process Value */
    if(attr->size) {
	if(encodeBase64((char*) attr->val, attr->size, &b64Data, &b64Size) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_AttrToJson: encodeBase64() failed\n");
#endif
	    return NULL;
	}
    }
    else {
	b64Data = strdup("");
	b64Size = sizeof("");
    }
    if(b64Data) {
	json_object_object_add(json, "Val", json_object_new_string(b64Data));
	if(freeBase64(&b64Data) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_AttrToJson: freeBase64() failed\n");
#endif
	}
    }
    else {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_AttrToJson: b64Data must not be NULL\n");
#endif
	return NULL;
    }

    return json;

}

extern json_object* custos_AttrReqToJson(const custosAttrReq_t* attrreq) {

    json_object* json = NULL;

    /* Validate Input */
    if(!attrreq) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_AttrReqToJson: 'attrreq' must not be NULL\n");
#endif
	return NULL;
    }

    /* Process Top Level Object */
    json = custos_AttrToJson(attrreq->attr);
    if(!json) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_AttrReqToJson: custos_AttrToJson() failed\n");
#endif
	return NULL;
    }
    json_object_object_add(json, "Echo",  json_object_new_boolean(attrreq->echo));

    return json;

}

extern json_object* custos_KeyToJson(const custosKey_t* key) {

    json_object* json = NULL;
    char uuidstr[37]  = "";
    char*  b64Data = NULL;
    size_t b64Size = 0;

    /* Validate Input */
    if(!key) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_KeyToJson: 'key' must not be NULL\n");
#endif
	return NULL;
    }

    /* Process Top Level Object */
    json = json_object_new_object();
    if(!json) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_KeyToJson: json_object_new_object() failed\n");
#endif
	return NULL;
    }
    uuid_unparse(key->uuid, uuidstr);
    json_object_object_add(json, "UUID", json_object_new_string(uuidstr));
    json_object_object_add(json, "Revision", json_object_new_int64(key->revision));

    /* Process Value */
    if(key->size) {
	if(encodeBase64((char*) key->val, key->size, &b64Data, &b64Size) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_KeyToJson: encodeBase64() failed\n");
#endif
	    return NULL;
	}
    }
    else {
	b64Data = strdup("");
	b64Size = sizeof("");
    }
    if(b64Data) {
	json_object_object_add(json, "Val", json_object_new_string(b64Data));
	if(freeBase64(&b64Data) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_KeyToJson: freeBase64() failed\n");
#endif
	}
    }
    else {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_KeyToJson: b64Data must not be NULL\n");
#endif
	return NULL;
    }

    return json;

}

extern json_object* custos_KeyReqToJson(const custosKeyReq_t* keyreq) {

    json_object* json = NULL;

    /* Validate Input */
    if(!keyreq) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_KeyReqToJson: 'keyreq' must not be NULL\n");
#endif
	return NULL;
    }

    /* Process Top Level Object */
    json = custos_KeyToJson(keyreq->key);
    if(!json) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_KeyReqToJson: custos_KeyToJson() failed\n");
#endif
	return NULL;
    }
    json_object_object_add(json, "Echo", json_object_new_boolean(keyreq->echo));

    return json;

}

extern json_object* custos_ReqToJson(const custosReq_t* req) {

    size_t i;
    json_object* keyreqobj  = NULL;
    json_object* keysobj    = NULL;
    json_object* attrreqobj = NULL;
    json_object* attrsobj   = NULL;
    json_object* reqobj     = NULL;
    json_object* checkobj   = NULL;
    json_object* json       = NULL;
    char*        md5sum     = NULL;

    /* Validate Args */
    if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_ReqToJson: 'req' must not be NULL\n");
#endif
	return NULL;
    }
    if(!(req->version)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_ReqToJson: 'req->version' must not be NULL\n");
#endif
	return NULL;
    }
    if(!(req->target)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_ReqToJson: 'req->version' must not be NULL\n");
#endif
	return NULL;
    }

    /* Process Attrs */
    attrsobj = json_object_new_array();
    if(!attrsobj) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_ReqToJson: json_object_new_array() failed\n");
#endif
	return NULL;
    }
    for(i = 0; i < req->num_attrs; i++) {
	attrreqobj = custos_AttrReqToJson(req->attrs[i]);
	if(!attrreqobj) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_ReqToJson: custos_AttrReqToJson() failed\n");
#endif
	    return NULL;
	}
	json_object_array_add(attrsobj, attrreqobj);
    }

    /* Process Keys */
    keysobj = json_object_new_array();
    if(!keysobj) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_ReqToJson: json_object_new_array() failed\n");
#endif
	return NULL;
    }
    for(i = 0; i < req->num_keys; i++) {
	keyreqobj = custos_KeyReqToJson(req->keys[i]);
	if(!keyreqobj) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_ReqToJson: custos_KeyReqToJson() failed\n");
#endif
	    return NULL;
	}
	json_object_array_add(keysobj, keyreqobj);
    }

    /* Process Request */
    reqobj = json_object_new_object();
    if(!reqobj) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_ReqToJson: json_object_new_object() failed\n");
#endif
	return NULL;
    }
    json_object_object_add(reqobj, "Target",  json_object_new_string(req->target));
    json_object_object_add(reqobj, "Version", json_object_new_string(req->version));
    json_object_object_add(reqobj, "ReqID",   json_object_new_string(""));
    json_object_object_add(reqobj, "Attrs",   attrsobj);
    json_object_object_add(reqobj, "Keys",    keysobj);

    /* Process Checksums */
    checkobj = json_object_new_object();
    if(!checkobj) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_ReqToJson: json_object_new_object() failed\n");
#endif
	return NULL;
    }
    md5sum = hashMD5(json_object_to_json_string(reqobj), 0);
    json_object_object_add(checkobj, "md5", json_object_new_string(md5sum));
    freeHash(&md5sum);

    /* Process Top Level Object */
    json = json_object_new_object();
    if(!json) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_ReqToJson: json_object_new_object() failed\n");
#endif
	return NULL;
    }
    json_object_object_add(json, "Request", reqobj);
    json_object_object_add(json, "Checksums", checkobj);

    return json;

}

extern custosRes_t* custos_JsonToRes(json_object* resjson) {

    int len;
    int i;
    json_object*      resobj     = NULL;
    json_object*      checkobj   = NULL;
    json_object*      attrsobj   = NULL;
    json_object*      keysobj    = NULL;
    json_object*      attrresobj = NULL;
    char*             sourceStr  = NULL;
    char*             versionStr = NULL;
    char*             reqidStr   = NULL;
    char*             residStr   = NULL;
    char*             statusStr  = NULL;
    custosAttrRes_t*  attrres    = NULL;
    custosResStatus_t status     = CUS_RESSTAT_MAX;
    custosRes_t*      res        = NULL;

    /* Process Top Level JSON */
    if(json_safe_get(resjson, json_type_object, "Response", &resobj) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToRes: json_safe_get(Response) failed\n");
#endif
	return NULL;
    }
    if(json_safe_get(resjson, json_type_object, "Checksums", &checkobj) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToRes: json_safe_get(Checksums) failed\n");
#endif
	return NULL;
    }

    /* Process Response Object */
    if(json_safe_get(resobj, json_type_string, "Source", &sourceStr) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToRes: json_safe_get(source) failed\n");
#endif
	return NULL;
    }
    if(json_safe_get(resobj, json_type_string, "Version", &versionStr) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToRes: json_safe_get(version) failed\n");
#endif
	return NULL;
    }
    if(strcmp(versionStr, CUS_VERSION) != 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToRes: version mismatch\n");
#endif
	return NULL;
    }
    if(json_safe_get(resobj, json_type_string, "ResID", &residStr) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToRes: json_safe_get(ResID) failed\n");
#endif
	return NULL;
    }
    if(json_safe_get(resobj, json_type_string, "ReqID", &reqidStr) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToRes: json_safe_get(ReqID) failed\n");
#endif
	return NULL;
    }
    // TODO Validate ReqID
    if(json_safe_get(resobj, json_type_string, "Status", &statusStr) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToRes: json_safe_get(Status) failed\n");
#endif
	return NULL;
    }
    if((status = custos_StrToResStatus(statusStr)) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToRes: custos_StrToResStatus() failed\n");
#endif
	return NULL;
    }
    if(json_safe_get(resobj, json_type_array, "Attrs", &attrsobj) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToRes: json_safe_get(Attrs) failed\n");
#endif
	return NULL;
    }
    if(json_safe_get(resobj, json_type_array, "Keys", &keysobj) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToRes: json_safe_get(Keys) failed\n");
#endif
	return NULL;
    }

    /* Setup Response */
    res = custos_createRes(status, sourceStr);
    if(!res) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToRes: custos_createRes() failed\n");
#endif
	return NULL;
    }

    /* Process Attrs */
    len = json_object_array_length(attrsobj);
    for(i=0; i < len; i++) {
	/* Extract AttrRes */
	attrresobj = json_object_array_get_idx(attrsobj, i);
	if(!attrresobj) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_JsonToRes: json_object_array_get_idx() failed\n");
#endif
	    return NULL;
	}
	/* Process AttrRes */
	attrres = custos_JsonToAttrRes(attrresobj);
	if(!attrres) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_JsonToRes: custos_JsonToAttrRes() failed\n");
#endif
	    return NULL;
	}
	/* Add AttrRes */
	if(custos_updateResAddAttrRes(res, attrres) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_JsonToRes: custos_updateResAddAttrRes() failed\n");
#endif
	    return NULL;
	}
    }

    /* Cleanup */
    free(sourceStr);
    free(versionStr);
    free(residStr);
    free(reqidStr);
    free(statusStr);

    /* Return */
    return res;

}

extern custosAttrRes_t* custos_JsonToAttrRes(json_object* attrresjson) {

    json_object*       attrobj   = NULL;
    char*              statusStr = NULL;
    bool               echo      = false;
    custosAttrStatus_t status    = CUS_ATTRSTAT_MAX;
    custosAttr_t*      attr      = NULL;
    custosAttrRes_t*   attrres   = NULL;

    /* Process Top Level JSON */
    if(json_safe_get(attrresjson, json_type_boolean, "Echo", &echo) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToAttrRes: json_safe_get(Echo) failed\n");
#endif
	return NULL;
    }
    if(json_safe_get(attrresjson, json_type_string, "Status", &statusStr) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToAttrRes: json_safe_get(Status) failed\n");
#endif
	return NULL;
    }
    if((status = custos_StrToAttrStatus(statusStr)) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToAttrRes: custos_StrToAttrResStatus() failed\n");
#endif
	return NULL;
    }

    /* Setup AttrRes */
    attrres = custos_createAttrRes(status, echo);
    if(!attrres) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToAttrRes: custos_createAttrRes() failed\n");
#endif
	return NULL;
    }

    /* Process Attr */
    attrobj = attrresjson;
    attr = custos_JsonToAttr(attrobj);
    if(!attr) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToAttrRes: custos_JsonToAttr() failed\n");
#endif
	return NULL;
    }

    /* Add Attr */
    if(custos_updateAttrResAddAttr(attrres, attr) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToAttrRes: custos_updateAttrResAddAttr() failed\n");
#endif
	return NULL;
    }

    /* Cleanup */
    free(statusStr);

    /* Return */
    return attrres;

}

extern custosAttr_t* custos_JsonToAttr(json_object* attrjson) {

    char*              classStr = NULL;
    char*              typeStr  = NULL;
    custosAttrClass_t  class    = CUS_ATTRCLASS_MAX;
    custosAttrType_t   type     = CUS_ATTRTYPE_MAX;
    int64_t            index    = 0;
    char*              valueStr = NULL;
    uint8_t*           value    = NULL;
    size_t             size     = 0;
    custosAttr_t*      attr     = NULL;

    /* Process Top Level JSON */
    if(json_safe_get(attrjson, json_type_string, "Class", &classStr) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToAttr: json_safe_get(Class) failed\n");
#endif
	return NULL;
    }
    if((class = custos_StrToAttrClass(classStr)) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToAttr: custos_StrToAttrClass() failed\n");
#endif
	return NULL;
    }
    free(classStr);
    classStr = NULL;
    if(json_safe_get(attrjson, json_type_string, "Type", &typeStr) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToAttr: json_safe_get(Type) failed\n");
#endif
	return NULL;
    }
    if((type = custos_StrToAttrType(class, typeStr)) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToAttr: custos_StrToAttrType() failed\n");
#endif
	return NULL;
    }
    free(typeStr);
    typeStr = NULL;
    if(json_safe_get(attrjson, json_type_int, "Index", &index) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToAttr: json_safe_get(Index) failed\n");
#endif
	return NULL;
    }
    if(json_safe_get(attrjson, json_type_string, "Value", &valueStr) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToAttr: json_safe_get(Value) failed\n");
#endif
	return NULL;
    }
    if(strcmp(valueStr, "") == 0) {
	value = NULL;
	size = 0;
    }
    else {
	if(decodeBase64(valueStr, strlen(valueStr), (char**) &value, &size) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_JsonToAttr: decodeBase64() failed\n");
#endif
	    return NULL;
	}
    }
    free(valueStr);
    valueStr = NULL;

    /* Setup Attr */
    attr = custos_createAttr(class, type, index, size, value);
    if(!attr) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToAttr: custos_createAttr() failed\n");
#endif
	return NULL;
    }

    /* Cleanup */
    if(value) {
	if(freeBase64((char**) &value) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_JsonToAttr: freeBase64() failed\n");
#endif
	}
    }

    /* Return */
    return attr;

}

extern custosKey_t* custos_JsonToKey(json_object* keyjson) {

    char*        uuidStr  = NULL;
    uuid_t       uuid;
    int64_t      revision = 0;
    char*        valueStr = NULL;
    uint8_t*     value    = NULL;
    size_t       size     = 0;
    custosKey_t* key      = NULL;

    /* Process Top Level JSON */
    if(json_safe_get(keyjson, json_type_string, "UUID", &uuidStr) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToKey: json_safe_get(UUID) failed\n");
#endif
	return NULL;
    }
    if(uuid_parse(uuidStr, uuid) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToKey: uuid_parse() failed\n");
#endif
	return NULL;
    }
    free(uuidStr);
    uuidStr = NULL;
    if(json_safe_get(keyjson, json_type_int, "Revision", &revision) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToKey: json_safe_get(Revision) failed\n");
#endif
	return NULL;
    }
    if(json_safe_get(keyjson, json_type_string, "Value", &valueStr) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToKey: json_safe_get(Value) failed\n");
#endif
	return NULL;
    }
    if(strcmp(valueStr, "") == 0) {
	value = NULL;
	size = 0;
    }
    else {
	if(decodeBase64(valueStr, strlen(valueStr), (char**) &value, &size) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_JsonToKey: decodeBase64() failed\n");
#endif
	    return NULL;
	}
    }
    free(valueStr);
    valueStr = NULL;

    /* Setup Key */
    key = custos_createKey(uuid, revision, size, value);
    if(!key) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_JsonToKey: custos_createKey() failed\n");
#endif
	return NULL;
    }

    /* Cleanup */
    if(value) {
	if(freeBase64((char**) &value) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_JsonToKey: freeBase64() failed\n");
#endif
	}
    }

    /* Return */
    return key;

}

/********* custosAttr Functions *********/

extern custosAttr_t* custos_createAttr(const custosAttrClass_t class,
				       const custosAttrType_t type,
				       const size_t index,
				       const size_t size, const uint8_t* val) {

    /* Local vars */
    custosAttr_t* attr = NULL;

    /* Input Invariant Check */
    if(class >= CUS_ATTRCLASS_MAX) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createAttr: 'class' must not be less than %d\n",
		CUS_ATTRCLASS_MAX);
#endif
	errno = EINVAL;
	return NULL;
    }
    if((type >= CUS_ATTRTYPE_EXP_MAX) && (type >= CUS_ATTRTYPE_IMP_MAX)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createAttr: 'type' must not be greater than %d or %d\n",
		CUS_ATTRTYPE_EXP_MAX, CUS_ATTRTYPE_IMP_MAX);
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
    attr->class = class;
    attr->type = type;
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

extern custosAttr_t* custos_duplicateAttr(const custosAttr_t* attr, bool echo) {

    /* Local Vars */
    custosAttr_t* out = NULL;

    /* Input Invariant Check */
    if(!attr) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_duplicateAttr: 'attr' must not be NULL\n");
#endif
	return NULL;
    }

    if(echo) {
	out = custos_createAttr(attr->class, attr->type,
				attr->index, attr->size, attr->val);
    }
    else {
	out = custos_createAttr(attr->class, attr->type,
				attr->index, 0, NULL);
    }
    if(!out) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_duplicateAttr: custos_createAttr() failed\n");
	perror(         "-------------------------->");
#endif
	return NULL;
    }

    return out;

}

extern int custos_updateAttr(custosAttr_t* attr,
			     const custosAttrClass_t class,
			     const custosAttrType_t type,
			     const size_t index,
			     const size_t size, const uint8_t* val) {

    /* Input Invariant Check */
    if(!attr) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateAttr: 'attr' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(class >= CUS_ATTRCLASS_MAX) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateAttr: 'class' must not be less than %d\n",
		CUS_ATTRCLASS_MAX);
#endif
	return -EINVAL;
    }
    if((type >= CUS_ATTRTYPE_EXP_MAX) && (type >= CUS_ATTRTYPE_IMP_MAX)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createAttr: 'type' must not be greater than %d or %d\n",
		CUS_ATTRTYPE_EXP_MAX, CUS_ATTRTYPE_IMP_MAX);
#endif
	return -EINVAL;
    }

    /* Update Meta Fields */
    attr->class = class;
    attr->type = type;
    attr->index = index;

    /* Free old value */
    if(attr->val) {
	free(attr->val);
	attr->val = NULL;
	attr->size = 0;
    }

    /* Set new value */
    if(size > 0) {
	/* Validate val */
	if(!val) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_updateAttr: 'val' can't be null when size non-zero\n");
#endif
	    return -EINVAL;
	}
	/* Create and Set New Attribute */
	attr->size = size;
	attr->val = malloc(attr->size);
	if(!(attr->val)) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_updateAttr: malloc(attr->size) failed\n");
	    perror(         "----------------------->");
#endif
	    return -errno;
	}
	memcpy(attr->val, val, attr->size);
    }

    return RETURN_SUCCESS;

}

extern const char* custos_AttrClassToStr(const custosAttrClass_t class) {

    switch(class) {
    case CUS_ATTRCLASS_IMPLICIT:
	return CUS_ATTRCLASS_IMPLICIT_STR;
    case CUS_ATTRCLASS_EXPLICIT:
	return CUS_ATTRCLASS_EXPLICIT_STR;
    default:
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_AttrClassToStr: Unrecognized class\n");
#endif
	return NULL;
    }

}

extern custosAttrClass_t custos_StrToAttrClass(const char* str) {

    if(!str) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_StrToAttrClass: 'str' must not be NULL\n");
#endif
	return -EINVAL;
    }

    if(strcmp(str, CUS_ATTRCLASS_IMPLICIT_STR) == 0) {
	return CUS_ATTRCLASS_IMPLICIT;
    }
    else if (strcmp(str, CUS_ATTRCLASS_EXPLICIT_STR) == 0) {
	return CUS_ATTRCLASS_EXPLICIT;
    }
    else {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_StrToAttrClass: unrecognized class\n");
#endif
	return -EPERM;
    }

}

extern const char* custos_AttrTypeToStr(const custosAttrClass_t class,
					const custosAttrType_t type) {

	switch(class) {
	case CUS_ATTRCLASS_IMPLICIT:
	switch(type) {
	case CUS_ATTRTYPE_IMP_SOURCEIP:
	    return CUS_ATTRTYPE_IMP_SOURCEIP_STR;
	default:
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_AttrTypeToStr: unrecognized implicit type\n");
#endif
	    return NULL;
	}
    case CUS_ATTRCLASS_EXPLICIT:
	switch(type) {
	case CUS_ATTRTYPE_EXP_PSK:
	    return CUS_ATTRTYPE_EXP_PSK_STR;
	default:
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_AttrTypeToStr: unrecognized explicit type\n");
#endif
	    return NULL;
	}
    default:
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_AttrTypeToStr: unrecognized class\n");
#endif
	return NULL;
    }

    }

extern custosAttrType_t custos_StrToAttrType(const custosAttrClass_t class,
					     const char* str) {

    if(!str) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_StrToAttrType: 'str' must not be NULL\n");
#endif
	return -EINVAL;
    }

    switch(class) {
    case CUS_ATTRCLASS_IMPLICIT:
	if(strcmp(str, CUS_ATTRTYPE_IMP_SOURCEIP_STR) == 0) {
	    return CUS_ATTRTYPE_IMP_SOURCEIP;
	}
	else {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_StrToAttrType: unrecognized implicit type\n");
#endif
	    return -EPERM;
	}
    case CUS_ATTRCLASS_EXPLICIT:
	if(strcmp(str, CUS_ATTRTYPE_EXP_PSK_STR) == 0) {
	    return CUS_ATTRTYPE_EXP_PSK;
	}
	else {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_StrToAttrType: unrecognized explicit type\n");
#endif
	    return -EPERM;
	}
    default:
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_StrToAttrType: unrecognized class\n");
#endif
	return -EPERM;
    }

}

/********* custosKey Functions *********/

extern custosKey_t* custos_createKey(const uuid_t uuid,
				     const uint64_t revision,
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
	fprintf(stderr, "ERROR custos_createKey: malloc(key) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }
    memset(key, 0, sizeof(*key));

    /* Populate */
    uuid_copy(key->uuid, uuid);
    key->revision = revision;
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
    key = NULL;
    *keyp = NULL;

    return RETURN_SUCCESS;

}

extern custosKey_t* custos_duplicateKey(const custosKey_t* key, bool echo) {

    /* Local Vars */
    custosKey_t* out = NULL;

    /* Input Invariant Check */
    if(!key) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_duplicateKey: 'key' must not be NULL\n");
#endif
	errno = EINVAL;
	return NULL;
    }

    if(echo) {
	out = custos_createKey(key->uuid, key->revision, key->size, key->val);
    }
    else {
	out = custos_createKey(key->uuid, key->revision, 0, NULL);
    }
    if(!out) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_duplicateKeyr: custos_createKey() failed\n");
	perror(         "-------------------------->");
#endif
	return NULL;
    }

    return out;

}

extern int custos_updateKey(custosKey_t* key,
			    const uuid_t uuid,
			    const uint64_t revision,
			    const size_t size, const uint8_t* val) {

    /* Input Invariant Check */
    if(!key) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateKey: 'key' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(uuid_is_null(uuid)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateKey: 'uuid' must not be null\n");
#endif
	return -EINVAL;
    }

    /* Update Meta Fields */
    uuid_copy(key->uuid, uuid);
    key->revision = revision;

    /* Free old value */
    if(key->val) {
	free(key->val);
	key->val = NULL;
	key->size = 0;
    }

    /* Set new value */
    if(size > 0) {
	/* Validate val */
	if(!val) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_updateKey: 'val' can't be null when size non-zero\n");
#endif
	    return -EINVAL;
	}
	/* Create and Set New Attribute */
	key->size = size;
	key->val = malloc(key->size);
	if(!(key->val)) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_updateKey: malloc(key->size) failed\n");
	    perror(         "---------------------->");
#endif
	    return -errno;
	}
	memcpy(key->val, val, key->size);
    }

    return RETURN_SUCCESS;

}

/********* custosAttrReq Functions *********/

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
    attrreq->attr = NULL;

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

extern int custos_updateAttrReqAddAttr(custosAttrReq_t* attrreq, custosAttr_t* attr) {

    /* Input Invariant Check */
    if(!attrreq) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateAttrReqAddAttr: 'attrreq' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!attr) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateAttrReqAddAttr: 'attr' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Add Attr to Request*/
    attrreq->attr = attr;

    return RETURN_SUCCESS;

}

/********* custosKeyReq Functions *********/

extern custosKeyReq_t* custos_createKeyReq(const bool echo) {

    /* Local vars */
    custosKeyReq_t* keyreq = NULL;

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

    /* Populate */
    keyreq->echo = echo;
    keyreq->key = NULL;

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

    /* Check and Free Optional Members */
    if(keyreq->key) {
	if(custos_destroyKey(&(keyreq->key)) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_destroyKeyReq: custos_destroyKey() failed\n");
#endif
	}
    }

    /* Free Struct */
    free(keyreq);
    keyreq = NULL;
    *keyreqp = NULL;

    return RETURN_SUCCESS;

}

extern int custos_updateKeyReqAddKey(custosKeyReq_t* keyreq, custosKey_t* key) {

    /* Input Invariant Check */
    if(!keyreq) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateKeyReqAddKey: 'keyreq' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!key) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateKeyReqAddKey: 'key' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Add Attr to Request*/
    keyreq->key = key;

    return RETURN_SUCCESS;

}

/********* custosAttrRes Functions *********/

extern custosAttrRes_t* custos_createAttrRes(const custosAttrStatus_t status, const bool echo) {

    /* Local vars */
    custosAttrRes_t* attrres = NULL;

    /* Initialize Vars */
    attrres = malloc(sizeof(*attrres));
    if(!attrres) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createAttrRes: malloc(attrres) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }
    memset(attrres, 0, sizeof(*attrres));

    /* Populate */
    attrres->status = status;
    attrres->echo = echo;
    attrres->attr = NULL;

    return attrres;

}

extern int custos_destroyAttrRes(custosAttrRes_t** attrresp) {

    /* Local Vars */
    custosAttrRes_t* attrres;

    /* Input Invariant Check */
    if(!attrresp) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyAttrRes: 'attrresp' must not be NULL\n");
#endif
	return -EINVAL;
    }
    attrres = *attrresp;

    if(!attrres) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyAttrRes: 'attrres' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Check and Free Optional Members */
    if(attrres->attr) {
	if(custos_destroyAttr(&(attrres->attr)) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_destroyAttrRes: custos_destroyAttr() failed\n");
#endif
	}
    }

    /* Free Struct */
    free(attrres);
    attrres = NULL;
    *attrresp = NULL;

    return RETURN_SUCCESS;

}

extern int custos_updateAttrResAddAttr(custosAttrRes_t* attrres, custosAttr_t* attr) {

    /* Input Invariant Check */
    if(!attrres) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateAttrResAddAttr: 'attrres' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!attr) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateAttrResAddAttr: 'attr' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Add Attr to Request*/
    attrres->attr = attr;

    return RETURN_SUCCESS;

}

extern const char* custos_AttrStatusToStr(const custosAttrStatus_t status) {

    switch(status) {
    case CUS_ATTRSTAT_ACCEPTED:
	return CUS_ATTRSTAT_ACCEPTED_STR;
    case CUS_ATTRSTAT_DENIED:
	return CUS_ATTRSTAT_DENIED_STR;
    case CUS_ATTRSTAT_REQUIRED:
	return CUS_ATTRSTAT_REQUIRED_STR;
    case CUS_ATTRSTAT_OPTIONAL:
	return CUS_ATTRSTAT_OPTIONAL_STR;
    case CUS_ATTRSTAT_IGNORED:
	return CUS_ATTRSTAT_IGNORED_STR;
    default:
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_AttrStatusToStr: unrecognized status\n");
#endif
	return NULL;
    }

}

extern custosAttrStatus_t custos_StrToAttrStatus(const char* str) {

    if(!str) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_StrToAttrStatus: 'str' must not be NULL\n");
#endif
	return -EINVAL;
    }

    if(strcmp(str, CUS_ATTRSTAT_ACCEPTED_STR) == 0) {
	return CUS_ATTRSTAT_ACCEPTED;
    }
    else if (strcmp(str, CUS_ATTRSTAT_DENIED_STR) == 0) {
	return CUS_ATTRSTAT_DENIED;
    }
    else if (strcmp(str, CUS_ATTRSTAT_REQUIRED_STR) == 0) {
	return CUS_ATTRSTAT_REQUIRED;
    }
    else if (strcmp(str, CUS_ATTRSTAT_OPTIONAL_STR) == 0) {
	return CUS_ATTRSTAT_OPTIONAL;
    }
    else if (strcmp(str, CUS_ATTRSTAT_IGNORED_STR) == 0) {
	return CUS_ATTRSTAT_IGNORED;
    }
    else {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_StrToAttrStatus: unrecognized status\n");
#endif
	return -EPERM;
    }

}

/********* custosKeyReq Functions *********/

extern custosKeyRes_t* custos_createKeyRes(const custosKeyStatus_t status, const bool echo) {

    /* Local vars */
    custosKeyRes_t* keyres = NULL;

    /* Initialize Vars */
    keyres = malloc(sizeof(*keyres));
    if(!keyres) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createKeyRes: malloc(keyres) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }
    memset(keyres, 0, sizeof(*keyres));

    /* Populate */
    keyres->status = status;
    keyres->echo = echo;
    keyres->key = NULL;

    return keyres;

}

extern int custos_destroyKeyRes(custosKeyRes_t** keyresp) {

    /* Local Vars */
    custosKeyRes_t* keyres;

    /* Input Invariant Check */
    if(!keyresp) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyKeyRes: 'keyresp' must not be NULL\n");
#endif
	return -EINVAL;
    }
    keyres = *keyresp;

    if(!keyres) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyKeyRes: 'keyres' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Check and Free Optional Members */
    if(keyres->key) {
	if(custos_destroyKey(&(keyres->key)) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_destroyKeyRes: custos_destroyKey() failed\n");
#endif
	}
    }

    /* Free Struct */
    free(keyres);
    keyres = NULL;
    *keyresp = NULL;

    return RETURN_SUCCESS;

}

extern int custos_updateKeyResAddKey(custosKeyRes_t* keyres, custosKey_t* key) {

    /* Input Invariant Check */
    if(!keyres) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateKeyResAddKey: 'keyres' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!key) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateKeyResAddKey: 'key' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Add Attr to Request*/
    keyres->key = key;

    return RETURN_SUCCESS;

}

/********* CustosReq Funcation*********/

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

extern int custos_updateReqAddAttrReq(custosReq_t* req, custosAttrReq_t* attrreq) {

    /* Local vars */
    int index = 0;

    /* Input Invariant Check */
    if(!req) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateReqAddAttrReq: 'req' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!attrreq) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateReqAddAttrReq: 'attrreq' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Add Attr to Array */
    index = req->num_attrs;
    if(index >= CUS_MAX_ATTRS) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateReqAddAttrReq: Max num_attrs exceeded\n");
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

/********* CustosRes Funcation*********/

extern custosRes_t* custos_getRes(const custosReq_t* req) {

    custosRes_t* res = NULL;
    custosAttr_t* attr = NULL;
    custosAttrRes_t* attrres = NULL;
    custosKey_t* key = NULL;
    custosKeyRes_t* keyres = NULL;
    size_t i = 0;
    bool psk = false;
    bool accept = false;
    json_object* reqJson = NULL;
    json_object* resJson = NULL;
    json_tokener* tok = NULL;
    char* reqJsonStr = NULL;
    char* reqUrlStr = NULL;
    char* fullReqUrl = NULL;
    HttpData_t resHttp;
    resHttp.size = 0;
    resHttp.data = NULL;

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

    /* Setup HTTP Get Request */
    reqJson = custos_ReqToJson(req);
    if(!reqJson) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_getRes: custos_ReqToJson() failed\n");
#endif
	return NULL;
    }
    reqJsonStr = strdup(json_object_to_json_string(reqJson));
    if(!reqJsonStr) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_getRes: strdup() failed\n");
#endif
	return NULL;
    }
    json_object_put(reqJson);
    reqJson = NULL;
    if(encodeURL(reqJsonStr, strlen(reqJsonStr), &reqUrlStr, NULL) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_getRes: encodeURL() failed\n");
#endif
	return NULL;
    }
    free(reqJsonStr);
    reqJsonStr = NULL;

    fullReqUrl = buildUrlGet(req->target, CUSTOS_ENDPOINT_KEYS,
			     CUSTOS_ENDPOINT_KEYS_REQ, reqUrlStr,
			     NULL);
    if(!fullReqUrl) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_getRes: buildUrlGet() failed\n");
#endif
	return NULL;
    }
    free(reqUrlStr);
    reqUrlStr = NULL;

    if(httpGet(fullReqUrl, &resHttp) != 200) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_getRes: httpGet() returned non-200 code\n");
#endif
	return NULL;
    }
    if(!resHttp.data) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_getRes: httpGet() returned NULL\n");
#endif
	return NULL;
    }
    free(fullReqUrl);
    fullReqUrl = NULL;

    tok = json_tokener_new();
    if(!tok) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_getRes: json_tokener_new() failed\n");
#endif
	return NULL;
    }
    resJson = json_tokener_parse_ex(tok, resHttp.data, strlen(resHttp.data));
    if(!resJson) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_getRes: json_tokener_parse() failed\n");
#endif
	return NULL;
    }
    json_tokener_free(tok);
    tok = NULL;
    free(resHttp.data);
    resHttp.data = NULL;
    res = custos_JsonToRes(resJson);
    if(!res) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_getRes: custos_JsonToRes() failed\n");
#endif
	return NULL;
    }
    json_object_put(resJson);
    resJson = NULL;

    /* ToDo: Process response from custos server */

    if(custos_printRes(res, 0, stderr) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_getRes: custos_printRes() failed\n");
#endif
	return NULL;
    }

    if(custos_destroyRes(&res) < 0) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_getRes: custos_destroyRes() failed\n");
#endif
	return NULL;
    }

    /* Start Dummy Response */
    res = custos_createRes(CUS_RESSTAT_ACCEPTED, req->target);

    /* Process Attributes */
    for(i = 0; i < req->num_attrs; i++) {
	attr = custos_duplicateAttr(req->attrs[i]->attr, req->attrs[i]->echo);
	if(!attr) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_getRes: custos_duplicateAttr() failed\n");
#endif
	    return NULL;
	}
	if(req->attrs[i]->attr->class == CUS_ATTRCLASS_EXPLICIT) {
	    if(req->attrs[i]->attr->type == CUS_ATTRTYPE_EXP_PSK) {
		psk = true;
		if (strcmp((char*) req->attrs[i]->attr->val, CUS_TEST_PSK_GOOD) == 0) {
		    attrres = custos_createAttrRes(CUS_ATTRSTAT_ACCEPTED, req->attrs[i]->echo);
		    accept = true;
		}
		else {
		    attrres = custos_createAttrRes(CUS_ATTRSTAT_DENIED, req->attrs[i]->echo);
		}
	    }
	    else {
		attrres = custos_createAttrRes(CUS_ATTRSTAT_IGNORED, req->attrs[i]->echo);
	    }
	}
	else {
	    attrres = custos_createAttrRes(CUS_ATTRSTAT_IGNORED, req->attrs[i]->echo);
	}

	if(!attrres) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_getRes: custos_createAttrRes() failed\n");
#endif
	    return NULL;
	}

	if(custos_updateAttrResAddAttr(attrres, attr) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_getRes: custos_updateAttrResAddAttr() failed\n");
#endif
	    return NULL;
	}

	if(custos_updateResAddAttrRes(res, attrres) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_getRes: custos_updateResAddAttrRes() failed\n");
#endif
	}

    }

    /* Add PSK Attribute if not sent */
    if(!psk) {

	attr = custos_createAttr(CUS_ATTRCLASS_EXPLICIT, CUS_ATTRTYPE_EXP_PSK,
				 0, 0, NULL);
	if(!attr) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_getRes: custos_createAttr() failed\n");
#endif
	    return NULL;
	}

	attrres = custos_createAttrRes(CUS_ATTRSTAT_REQUIRED, false);
	if(!attrres) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_getRes: custos_createAttrRes() failed\n");
#endif
	    return NULL;
	}

	if(custos_updateAttrResAddAttr(attrres, attr) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_getRes: custos_updateAttrResAddAttr() failed\n");
#endif
	    return NULL;
	}

	if(custos_updateResAddAttrRes(res, attrres) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_getRes: custos_updateResAddAttrRes() failed\n");
#endif
	}

    }

    /* Process Keys */
    for(i = 0; i < req->num_keys; i++) {
	key = custos_duplicateKey(req->keys[i]->key, req->keys[i]->echo);
	if(!key) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_getRes: custos_duplicateKey() failed\n");
#endif
	    return NULL;
	}
	if(accept) {
	    if(custos_updateKey(key, req->keys[i]->key->uuid, req->keys[i]->key->revision,
				(strlen(TEST_KEY) + 1), (uint8_t*) TEST_KEY) < 0) {
#ifdef DEBUG
		fprintf(stderr, "ERROR custos_getRes: custos_updateKey() failed\n");
#endif
		return NULL;
	    }
	    keyres = custos_createKeyRes(CUS_KEYSTAT_ACCEPTED, req->keys[i]->echo);
	}
	else {
	    if(custos_updateKey(key, req->keys[i]->key->uuid, req->keys[i]->key->revision,
				0, NULL) < 0) {
#ifdef DEBUG
		fprintf(stderr, "ERROR custos_getRes: custos_updateKey() failed\n");
#endif
		return NULL;
	    }
	    keyres = custos_createKeyRes(CUS_KEYSTAT_DENIED, false);
	}

	if(!keyres) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_getRes: custos_createKeyRes() failed\n");
#endif
	    return NULL;
	}

	if(custos_updateKeyResAddKey(keyres, key) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_getRes: custos_updateKeyResAddKey() failed\n");
#endif
	    return NULL;
	}

	if(custos_updateResAddKeyRes(res, keyres) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_getRes: custos_updateResAddKeyRes() failed\n");
#endif
	}

    }

    /* End Dummy Response */

    return res;
}

extern custosRes_t* custos_createRes(const custosResStatus_t status, const char* source) {

    custosRes_t* res = NULL;

    res = malloc(sizeof(*res));
    if(!res) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createRes: malloc(res) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }
    memset(res, 0, sizeof(*res));

    res->source = strdup(source);
    if(!(res->source)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createRes: strdup(source) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }

    res->version = strdup(CUS_VERSION);
    if(!(res->version)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_createRes: strdup(CUSTOS_VERSION) failed\n");
	perror(         "---------------------->");
#endif
	return NULL;
    }

    res->status = status;
    res->num_attrs = 0;
    res->num_keys = 0;

    return res;

}

extern int custos_destroyRes(custosRes_t** resp) {

    /* Local vars */
    uint i;
    custosRes_t* res;

    /* Input Invariant Check */
    if(!resp) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyRes: 'resp' must not be NULL\n");
#endif
	return -EINVAL;
    }
    res = *resp;

    if(!res) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyRes: 'res' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Check and Free Required Members */
    if(!(res->source)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyRes: 'res->source' must not be NULL\n");
#endif
    }
    free(res->source);

    if(!(res->version)) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_destroyRes: 'res->version' must not be NULL\n");
#endif
    }
    free(res->version);

    /* Check and Free Optional Members */
    /* Free Attrs */
    for(i = 0; i < res->num_attrs; i++) {
	if(custos_destroyAttrRes(&(res->attrs[i])) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_destroyRes: custos_destroyAttrRes() failed\n");
#endif
	}
    }
    res->num_attrs = 0;

    /* Free Keys */
    for(i = 0; i < res->num_keys; i++) {
	if(custos_destroyKeyRes(&(res->keys[i])) < 0) {
#ifdef DEBUG
	    fprintf(stderr, "ERROR custos_destroyRes: custos_destroyKeyRes() failed\n");
#endif
	}
    }
    res->num_keys = 0;

    /* Free Struct */
    free(res);
    res = NULL;
    *resp = NULL;

    return RETURN_SUCCESS;

}

extern int custos_updateResAddAttrRes(custosRes_t* res, custosAttrRes_t* attrres) {

    /* Local vars */
    int index = 0;

    /* Input Invariant Check */
    if(!res) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateResAddAttrRes: 'res' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!attrres) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateResAddAttrRes: 'attrres' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Add Attr to Array */
    index = res->num_attrs;
    if(index >= CUS_MAX_ATTRS) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateResAddAttrRes: Max num_attrs exceeded\n");
#endif
	return -ERANGE;
    }
    res->attrs[index] = attrres;
    res->num_attrs++;

    return index;

}

extern int custos_updateResAddKeyRes(custosRes_t* res, custosKeyRes_t* keyres) {

    /* Local vars */
    int index = 0;

    /* Input Invariant Check */
    if(!res) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateResAddKeyRes: 'res' must not be NULL\n");
#endif
	return -EINVAL;
    }
    if(!keyres) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateResAddKeyRes: 'keyres' must not be NULL\n");
#endif
	return -EINVAL;
    }

    /* Add Key to Array */
    index = res->num_keys;
    if(index >= CUS_MAX_KEYS) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_updateResAddKeyRes: Max num_keys exceeded\n");
#endif
	return -ERANGE;
    }
    res->keys[index] = keyres;
    res->num_keys++;

    return index;

}

extern const char* custos_ResStatusToStr(const custosResStatus_t status) {

    switch(status) {
    case CUS_RESSTAT_ACCEPTED:
	return CUS_RESSTAT_ACCEPTED_STR;
    case CUS_RESSTAT_DENIED:
	return CUS_RESSTAT_DENIED_STR;
    case CUS_RESSTAT_ERROR:
	return CUS_RESSTAT_ERROR_STR;
    default:
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_ResStatusToStr: unrecognized status\n");
#endif
	return NULL;
    }

}

extern custosResStatus_t custos_StrToResStatus(const char* str) {

    if(!str) {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_StrToResStatus: 'str' must not be NULL\n");
#endif
	return -EINVAL;
    }

    if(strcmp(str, CUS_RESSTAT_ACCEPTED_STR) == 0) {
	return CUS_RESSTAT_ACCEPTED;
    }
    else if (strcmp(str, CUS_RESSTAT_DENIED_STR) == 0) {
	return CUS_RESSTAT_DENIED;
    }
    else if (strcmp(str, CUS_RESSTAT_ERROR_STR) == 0) {
	return CUS_RESSTAT_ERROR;
    }
    else {
#ifdef DEBUG
	fprintf(stderr, "ERROR custos_StrToResStatus: unrecognized status\n");
#endif
	return -EPERM;
    }

}
