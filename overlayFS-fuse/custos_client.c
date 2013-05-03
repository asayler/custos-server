/* custos_client.c
 * 
 * custos_client interface implementation
 *
 * By Andy Sayler (www.andysayler.com)
 * Created  05/02/13
 *
 */

#include "custos_client.h"

#define RETURN_FAILURE -1
#define RETURN_SUCCESS  0

extern int custos_getkey(const custosReq_t* req, custosRes_t* res) {

    (void) req;
    (void) res;

    return RETURN_FAILURE;
}
