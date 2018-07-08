#include <memory.h>
#include "request.h"
#include "debug.h"
#include "sbuf.h"

//---------------------------[ http_request_init ]-----------------------------
// initializes the fields of an http_requsest_t structure
// private function used by http_request_alloc
//-----------------------------------------------------------------------------

static void http_request_init(http_request_t* self)
{
    memset(self, 0, sizeof(*self));
    sbuf_init(&self->sbuf);
    kvpbuf_init(&self->kvpbuf);
    self->sbuf_inc = HTTP_REQUEST_DEFAULT_SBUF_INC;
    self->kvpbuf_inc = HTTP_REQUEST_DEFAULT_KVPBUF_INC;
}

//---------------------------[ http_request_alloc ]----------------------------
// Allocates and initializes
//-----------------------------------------------------------------------------

http_request_t* http_request_alloc()
{
    http_request_t* ans = (http_request_t*)debug_realloc(0, sizeof(http_request_t));
    http_request_init(ans);
    return ans;
}

//---------------------------[ http_request_free ]-----------------------------
// deletes all resources and then frees up the memory
//-----------------------------------------------------------------------------

void http_request_free(http_request_t* self)
{
    sbuf_delete(&self->sbuf);
    kvpbuf_delete(&self->kvpbuf);
}
