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

typedef enum custosAttrID {
    CUS_ATTRID_PSK = 0,
    CUS_ATTRID_MAX,
} custosAttrID_t;

typedef struct custosReq {
    uuid_t       uuid;
    char*        uri;
    custosAttr_t attrs[CUS_ATTRID_MAX];
} custosReq_t;

typedef struct custosRes {
    uint8_t* key;
} custosRes_t;

extern custosReq_t* custos_createReq(const uuid_t uuid, const char* uri);
extern int custos_updateReq(custosReq_t* req, const custosAttrID_t id,
			    const void* value, const size_t size);
extern int custos_destroyReq(custosReq_t** req);

extern int custos_getkey(const custosReq_t* req, custosRes_t* res);

#endif
