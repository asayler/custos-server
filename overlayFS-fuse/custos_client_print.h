/* custos_client_print.h
 *
 * custos_client_print interface
 *
 * By Andy Sayler (www.andysayler.com)
 * Created  10/13
 *
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <uuid/uuid.h>

#include "custos_client.h"

#define CUS_PRINT_OFFSET 4

char* custos_stringifyVal(size_t size, uint8_t* val);

int custos_printAttr(custosAttr_t* attr, uint offset, FILE* stream);
int custos_printAttrReq(custosAttrReq_t* attrreq, uint offset, FILE* stream);
int custos_printAttrRes(custosAttrRes_t* attrres, uint offset, FILE* stream);

int custos_printKey(custosKey_t* key, uint offset, FILE* stream);
int custos_printKeyReq(custosKeyReq_t* keyreq, uint offset, FILE* stream);
int custos_printKeyRes(custosKeyRes_t* keyres, uint offset, FILE* stream);

int custos_printReq(custosReq_t* req, uint offset, FILE* stream);
int custos_printRes(custosRes_t* res, uint offset, FILE* stream);
