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
    custosAttrStatus_t status;
    size_t             size;
    uint8_t*           val;
} custosAttr_t;

typedef struct custosKey {
    uuid_t   uuid;
    size_t   size;
    uint8_t* val;
} custosKey_t;

typedef struct custosKeyReq {
    char*        uri;
    size_t       num_attrs;
    custosAttr_t attrs[CUS_MAX_ATTRS];
    size_t       num_keys;
    custosKey_t  keys[CUS_MAX_KEYS];
} custosKeyReq_t;

typedef struct custosKeyRes {
    custosResStatus_t status;
    size_t            num_attrs;
    custosAttr_t      attrs[CUS_MAX_ATTRS];
    size_t            num_keys;
    custosKey_t       keys[CUS_MAX_KEYS];
} custosKeyRes_t;

extern custosKey_t* custos_createKey(const uuid_t uuid);
extern int custos_destroyKey(custosKey_t** keyp);

extern custosKey_t* custos_createAttr(const custosAttrType_t type,
				      const custosAttrClass_t class,
				      const custosAttrID_t id,
				      const custosAttrStatus_t status,
				      const size_t size, const uint8_t* val);
extern int custos_destroyAttr(custosAttr_t** attrp);

extern custosKeyReq_t* custos_createKeyReq(const char* uri);
extern int custos_updateKeyReqKey(custosKeyReq_t* req, const custosKey_t* key);
extern int custos_updateKeyReqAttr(custosKeyReq_t* req, const custosAttr_t* attr);
extern int custos_destroyKeyReq(custosKeyReq_t** reqp);

extern custosKeyRes_t* custos_getKeyRes(const custosKeyReq_t* req);
extern int custos_destroyKeyRes(custosKeyRes_t** resp);

#endif
