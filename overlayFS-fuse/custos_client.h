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
#include <inttypes.h>

typedef struct custosReq {
    uint8_t uuid[16];
    char*   psk;
} custosReq_t;

typedef struct custosRes {
    uint8_t* key;
} custosRes_t;

extern int custos_getkey(const custosReq_t* req, custosRes_t* res);

#endif
