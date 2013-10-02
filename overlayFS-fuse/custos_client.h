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

#define CUS_TEST_PSK "password"
#define CUS_TEST_BADPSK "badpassword"

#define CUS_MAX_ATTRS 100
#define CUS_MAX_KEYS 100

#define CUS_VERSION "0.1-dev"

typedef enum custosResStatus {
    CUS_RESSTAT_ACCEPTED = 0,
    CUS_RESSTAT_DENIED,
    CUS_RESSTAT_ERROR,
    CUS_RESSTAT_MAX,
} custosResStatus_t;

typedef enum custosAttrStatus {
    CUS_ATTRSTAT_ACCEPTED = 0,
    CUS_ATTRSTAT_DENIED,
    CUS_ATTRSTAT_REQUIRED,
    CUS_ATTRSTAT_OPTIONAL,
    CUS_ATTRSTAT_IGNORED,
    CUS_ATTRSTAT_MAX
} custosAttrStatus_t;

typedef enum custosKeyStatus {
    CUS_KEYSTAT_ACCEPTED = 0,
    CUS_KEYSTAT_DENIED,
    CUS_KEYSTAT_MAX,
} custosKeyStatus_t;

typedef enum custosAttrType {
    CUS_ATTRTYPE_IMPLICIT = 0,
    CUS_ATTRTYPE_EXPLICIT,
    CUS_ATTRTYPE_MAX,
} custosAttrType_t;

typedef enum custosAttrClass {
    CUS_ATTRCLASS_DIRECT = 0,
    CUS_ATTRCLASS_NETWORKID,
    CUS_ATTRCLASS_CLIENTID,
    CUS_ATTRCLASS_MAX,
} custosAttrClass_t;

typedef enum custosAttrID {
    CUS_ATTRID_PSK = 0,
    CUS_ATTRID_SOURCEIP,
    CUS_ATTRID_CLIENTCERT,
    CUS_ATTRID_MAX,
} custosAttrID_t;

typedef struct custosAttr {
    custosAttrType_t   type;
    custosAttrClass_t  class;
    custosAttrID_t     id;
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
    uint64_t version;
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

extern custosAttrReq_t* custos_createAttrReq(const custosAttrType_t type,
					     const custosAttrClass_t class,
					     const custosAttrID_t id,
					     const size_t index,
					     const size_t size, const uint8_t* val,
					     const bool echo);
extern int custos_destroyAttrReq(custosAttrReq_t** attrreqp);

extern custosKeyReq_t* custos_createKeyReq(const uuid_t uuid,
					   const uint64_t version,
					   const size_t size, const uint8_t* val,
					   const bool echo);
extern int custos_destroyKeyReq(custosKeyReq_t** keyreqp);

extern custosReq_t* custos_createReq(const char* target);
/* extern int custos_destroyReq(custosReq_t** reqp); */

/* extern int custos_updateReqAddAttr(custosReq_t* req, const custosAttrReq_t* attr); */
/* extern int custos_updateReqAddKey(custosReq_t* req, const custosKeyReq_t* key); */

/*
extern custosRes_t* custos_getRes(const custosReq_t* req);
extern int custos_destroyRes(custosRes_t** resp);
*/

#endif
