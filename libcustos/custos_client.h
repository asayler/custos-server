/* custos_client.h
 *
 * custos_client interface - C bindings
 *
 * By Andy Sayler (www.andysayler.com)
 * Created  05/02/13
 *
 */

#ifndef CUSTOS_CLIENT_H
#define CUSTOS_CLIENT_H

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <uuid/uuid.h>
#include <json/json.h>

#include "custos_http.h"
#include "custos_json.h"

#define CUS_MAX_ATTRS 100
#define CUS_MAX_KEYS 100

#define CUS_VERSION "0.1-dev"

#define CUSTOS_ENDPOINT_KEYS "/keys"
//#define CUSTOS_ENDPOINT_KEYS "/custos/01-accept-response.json"
#define CUSTOS_ENDPOINT_KEYS_REQ "req"
#define CUSTOS_ENDPOINT_KEYS_CHK "chk"

typedef enum custosResStatus {
    CUS_RESSTAT_ACCEPTED = 0,
    CUS_RESSTAT_DENIED,
    CUS_RESSTAT_ERROR,
    CUS_RESSTAT_MAX
} custosResStatus_t;

#define CUS_RESSTAT_ACCEPTED_STR "accepted"
#define CUS_RESSTAT_DENIED_STR   "denied"
#define CUS_RESSTAT_ERROR_STR    "error"

typedef enum custosAttrStatus {
    CUS_ATTRSTAT_ACCEPTED = 0,
    CUS_ATTRSTAT_DENIED,
    CUS_ATTRSTAT_REQUIRED,
    CUS_ATTRSTAT_OPTIONAL,
    CUS_ATTRSTAT_IGNORED,
    CUS_ATTRSTAT_MAX
} custosAttrStatus_t;

#define CUS_ATTRSTAT_ACCEPTED_STR "accepted"
#define CUS_ATTRSTAT_DENIED_STR   "denied"
#define CUS_ATTRSTAT_REQUIRED_STR "required"
#define CUS_ATTRSTAT_OPTIONAL_STR "optional"
#define CUS_ATTRSTAT_IGNORED_STR  "ignored"

typedef enum custosKeyStatus {
    CUS_KEYSTAT_ACCEPTED = 0,
    CUS_KEYSTAT_DENIED,
    CUS_KEYSTAT_UNKNOWN,
    CUS_KEYSTAT_MAX
} custosKeyStatus_t;

#define CUS_KEYSTAT_ACCEPTED_STR "accepted"
#define CUS_KEYSTAT_DENIED_STR   "denied"
#define CUS_KEYSTAT_UNKNOWN_STR   "unknown"

typedef enum custosAttrClass {
    CUS_ATTRCLASS_IMPLICIT = 0,
    CUS_ATTRCLASS_EXPLICIT,
    CUS_ATTRCLASS_MAX
} custosAttrClass_t;

#define CUS_ATTRCLASS_IMPLICIT_STR "implicit"
#define CUS_ATTRCLASS_EXPLICIT_STR "explicit"

typedef enum custosAttrType {
    CUS_ATTRTYPE_EXP_PSK = 0,
    CUS_ATTRTYPE_EXP_MAX,
    CUS_ATTRTYPE_IMP_SOURCEIP = 0,
    CUS_ATTRTYPE_IMP_MAX
} custosAttrType_t;

#define CUS_ATTRTYPE_MAX CUS_ATTRTYPE_IMP_MAX
#define CUS_ATTRTYPE_EXP_PSK_STR        "psk"
#define CUS_ATTRTYPE_IMP_SOURCEIP_STR   "ip_source"

typedef struct custosAttr {
    custosAttrClass_t  class;
    custosAttrType_t   type;
    size_t             index;
    size_t             size;
    uint8_t*           val;
} custosAttr_t;

typedef struct custosAttrReq {
    bool               echo;
    custosAttr_t*      attr;
} custosAttrReq_t;

typedef struct custosAttrRes {
    custosAttrStatus_t status;
    bool               echo;
    custosAttr_t*      attr;
} custosAttrRes_t;

typedef struct custosKey {
    uuid_t   uuid;
    uint64_t revision;
    size_t   size;
    uint8_t* val;
} custosKey_t;

typedef struct custosKeyReq {
    bool              echo;
    custosKey_t*      key;
} custosKeyReq_t;

typedef struct custosKeyRes {
    custosKeyStatus_t status;
    bool              echo;
    custosKey_t*      key;
} custosKeyRes_t;

typedef struct custosReq {
    char*            target;
    char*            version;
    size_t           num_attrs;
    custosAttrReq_t* attrs[CUS_MAX_ATTRS];
    size_t           num_keys;
    custosKeyReq_t*  keys[CUS_MAX_KEYS];
} custosReq_t;

typedef struct custosRes {
    custosResStatus_t status;
    char*             source;
    char*             version;
    size_t            num_attrs;
    custosAttrRes_t*  attrs[CUS_MAX_ATTRS];
    size_t            num_keys;
    custosKeyRes_t*   keys[CUS_MAX_KEYS];
} custosRes_t;


/* custosAttr Functions */
extern custosAttr_t* custos_createAttr(const custosAttrClass_t class,
				       const custosAttrType_t type,
				       const size_t index,
				       const size_t size, const uint8_t* val);
extern int custos_destroyAttr(custosAttr_t** attrp);
extern custosAttr_t* custos_duplicateAttr(const custosAttr_t* attr, bool echo);
extern int custos_updateAttr(custosAttr_t* attr,
			     const custosAttrClass_t class,
			     const custosAttrType_t type,
			     const size_t index,
			     const size_t size, const uint8_t* val);

extern const char* custos_AttrClassToStr(const custosAttrClass_t class);
extern custosAttrClass_t custos_StrToAttrClass(const char* str);
extern const char* custos_AttrTypeToStr(const custosAttrClass_t class,
					const custosAttrType_t type);
extern custosAttrType_t custos_StrToAttrType(const custosAttrClass_t class,
					     const char* str);

/* custosKey Functions */
extern custosKey_t* custos_createKey(const uuid_t uuid,
				     const uint64_t revision,
				     const size_t size, const uint8_t* val);
extern int custos_destroyKey(custosKey_t** keyp);
extern custosKey_t* custos_duplicateKey(const custosKey_t* key, bool echo);
extern int custos_updateKey(custosKey_t* key,
			    const uuid_t uuid,
			    const uint64_t revision,
			    const size_t size, const uint8_t* val);

/* custosAttrReq Functions */
extern custosAttrReq_t* custos_createAttrReq(const bool echo);
extern int custos_destroyAttrReq(custosAttrReq_t** attrreqp);
extern int custos_updateAttrReqAddAttr(custosAttrReq_t* attrreq, custosAttr_t* attr);

/* custosKeyReq Functions */
extern custosKeyReq_t* custos_createKeyReq(const bool echo);
extern int custos_destroyKeyReq(custosKeyReq_t** keyreqp);
extern int custos_updateKeyReqAddKey(custosKeyReq_t* keyreq, custosKey_t* key);

/* custosAttrRes Functions */
extern custosAttrRes_t* custos_createAttrRes(const custosAttrStatus_t status, const bool echo);
extern int custos_destroyAttrRes(custosAttrRes_t** attrresp);
extern int custos_updateAttrResAddAttr(custosAttrRes_t* attrres, custosAttr_t* attr);

extern const char* custos_AttrStatusToStr(const custosAttrStatus_t status);
extern custosAttrStatus_t custos_StrToAttrStatus(const char* str);

/* custosKeyRes Functions */
extern custosKeyRes_t* custos_createKeyRes(const custosKeyStatus_t status, const bool echo);
extern int custos_destroyKeyRes(custosKeyRes_t** keyresp);
extern int custos_updateKeyResAddKey(custosKeyRes_t* keyres, custosKey_t* key);

extern const char* custos_KeyStatusToStr(const custosKeyStatus_t status);
extern custosKeyStatus_t custos_StrToKeyStatus(const char* str);

/* custosReq Functions */
extern custosReq_t* custos_createReq(const char* target);
extern int custos_destroyReq(custosReq_t** reqp);
extern int custos_updateReqAddAttrReq(custosReq_t* req, custosAttrReq_t* attrreq);
extern int custos_updateReqAddKeyReq(custosReq_t* req, custosKeyReq_t* keyreq);

/* custosRes Functions */
extern custosRes_t* custos_getRes(const custosReq_t* req);
extern custosRes_t* custos_createRes(const custosResStatus_t status, const char* source);
extern int custos_destroyRes(custosRes_t** resp);
extern int custos_updateResAddAttrRes(custosRes_t* res, custosAttrRes_t* attrres);
extern int custos_updateResAddKeyRes(custosRes_t* res, custosKeyRes_t* keyres);

extern const char* custos_ResStatusToStr(const custosResStatus_t status);
extern custosResStatus_t custos_StrToResStatus(const char* str);

/* JSON */
extern json_object* custos_AttrToJson(const custosAttr_t* attr);
extern json_object* custos_KeyToJson(const custosKey_t* key);

extern json_object* custos_AttrReqToJson(const custosAttrReq_t* attrreq);
extern json_object* custos_KeyReqToJson(const custosKeyReq_t* keyreq);

extern json_object* custos_ReqToJson(const custosReq_t* req);

extern custosAttr_t* custos_JsonToAttr(json_object* attrjson);
extern custosKey_t* custos_JsonToKey(json_object* keyjson);

extern custosAttrRes_t* custos_JsonToAttrRes(json_object* attrresjson);
extern custosKeyRes_t* custos_JsonToKeyRes(json_object* keyresjson);

extern custosRes_t* custos_JsonToRes(json_object* resjson);

#endif
