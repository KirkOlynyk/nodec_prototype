#include <memory.h>
#include "debug.h"
#include "utils.h"
#include "kvp.h"

//---------------------------[ kvpbuf_add ]------------------------------------
//-----------------------------------------------------------------------------

void kvpbuf_add(kvpbuf_t* self, const kvp_t* kvp, size_t kvp_inc)
{
    if (self->total <= self->used) {
        size_t total = self->total + kvp_inc;
        self->buffer = debug_realloc(self->buffer, sizeof(kvp_t) * total);
        self->total = total;
    }
    self->buffer[self->used] = *kvp;
    self->used += 1;
}

//---------------------------[ kvpbuf_delete ]---------------------------------
//-----------------------------------------------------------------------------

void kvpbuf_delete(kvpbuf_t* self)
{
    if (self->buffer) {
        debug_free(self->buffer);
        self->buffer = 0;
    }
}

//---------------------------[ kvpbuf_init ]-----------------------------------
//-----------------------------------------------------------------------------

void kvpbuf_init(kvpbuf_t* self)
{
    memset(self, 0, sizeof(self));
}
