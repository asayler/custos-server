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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <uuid/uuid.h>
#include <json/json.h>

#include "http-util.h"

#define CUS_TEST_PSK_GOOD "swordfish"

#define CUS_MAX_ATTRS 100
#define CUS_MAX_KEYS 100

#define CUS_VERSION "0.1-dev"

typedef enum custosResStatus {
    CUS_RESSTAT_ACCEPTED = 0,
    CUS_RESSTAT_DENIED,
    CUS_RESSTAT_ERROR,
    CUS_RESSTAT_MAX
} custosResStatus_t;

typedef enum custosAttrResStatus {
    CUS_ATTRSTAT_ACCEPTED = 0,
    CUS_ATTRSTAT_DENIED,
    CUS_ATTRSTAT_REQUIRED,
    CUS_ATTRSTAT_OPTIONAL,
    CUS_ATTRSTAT_IGNORED,
    CUS_ATTRSTAT_MAX
} custosAttrStatus_t;

typedef enum custosKeyResStatus {
    CUS_KEYSTAT_ACCEPTED = 0,
    CUS_KEYSTAT_DENIED,
    CUS_KEYSTAT_MAX
} custosKeyStatus_t;

typedef enum custosAttrClass {
    CUS_ATTRCLASS_IMPLICIT = 0,
    CUS_ATTRCLASS_EXPLICIT,
    CUS_ATTRCLASS_MAX
} custosAttrClass_t;

typedef enum custosAttrType {
    CUS_ATTRTYPE_EXP_PSK = 0,
    CUS_ATTRTYPE_EXP_MAX,
    CUS_ATTRTYPE_IMP_SOURCEIP = 0,
    CUS_ATTRTYPE_IMP_CLIENTCERT,
    CUS_ATTRTYPE_IMP_MAX
} custosAttrType_t;

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

/* custosKeyRes Functions */
extern custosKeyRes_t* custos_createKeyRes(const custosKeyStatus_t status, const bool echo);
extern int custos_destroyKeyRes(custosKeyRes_t** keyresp);
extern int custos_updateKeyResAddKey(custosKeyRes_t* keyres, custosKey_t* key);

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

#endif
