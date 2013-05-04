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

typedef struct custosAttr {
    void*  val;
    size_t size;
} custosAttr_t;

typedef enum custosAttrStatus {
    CUS_ATTRSTAT_GOOD = 0,
    CUS_ATTRSTAT_BAD,
    CUS_ATTRSTAT_REQ,
    CUS_ATTRSTAT_OPT,
    CUS_ATTRSTAT_NA,
} custosAttrStatus_t;

typedef enum custosAttrID {
    CUS_ATTRID_PSK = 0,
    CUS_ATTRID_MAX,
} custosAttrID_t;

typedef struct custosKeyReq {
    uuid_t       uuid;
    char*        uri;
    custosAttr_t attrs[CUS_ATTRID_MAX];
} custosKeyReq_t;

typedef struct custosRes {
    uint8_t*           key;
    size_t             size;
    custosAttrStatus_t attrStat[CUS_ATTRID_MAX];
} custosRes_t;

extern custosKeyReq_t* custos_createKeyReq(const uuid_t uuid, const char* uri);
extern int custos_updateKeyReq(custosKeyReq_t* req, const custosAttrID_t id,
			    const void* value, const size_t size);
extern int custos_destroyKeyReq(custosKeyReq_t** req);

extern int custos_getkey(const custosKeyReq_t* req, custosRes_t* res);

#endif
