#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <memory.h>
#include "debug.h"

//---------------------------[ debug_free ]------------------------------------
//-----------------------------------------------------------------------------

void debug_free(void* block)
{
    printf("free(%p) -> void\n", block);
    free(block);
}

//---------------------------[ debug_realloc ]---------------------------------
//-----------------------------------------------------------------------------

void* debug_realloc(void* block, size_t size)
{
    void* ans = realloc(block, size);
    printf("realloc(%p, %zu) -> %p\n", block, size, ans);
    return ans;
}

//---------------------------[ hex_dump ]--------------------------------------
//-----------------------------------------------------------------------------

void hexDump(const void *addr, size_t len, const char* prefix)
{
    if (prefix)
    {
        printf("%s\n", prefix);
    }
    size_t i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;
    for (i = 0; i < len; i++) {
        if ((i % 16) == 0) {
            if (i != 0)
                printf("  %s\n", buff);
            printf("%p ", pc + i);
        }
        unsigned char c = pc[i];
        printf(" %02x", c);
        size_t imod = i % 16;
        buff[imod] = isprint(c) ? c : '.';
        buff[imod + 1] = 0;
    }
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }
    printf("  %s\n", buff);
}

//---------------------------[ debug_memcpy ]----------------------------------
//-----------------------------------------------------------------------------

void debug_memcpy(void* dst, const void* src, size_t size)
{
    printf("debug_memcpy(%p, %p, %zu)\n", dst, src, size);
    memcpy(dst, src, size);
}

//---------------------------[ pause ]-----------------------------------------
// Print the message and then wait for the user to press ENTER
//-----------------------------------------------------------------------------

void pause(const char* msg)
{
    printf("%s\n", msg);
    char buf[16];
    gets_s(buf, sizeof(buf));
}

//---------------------------[ print_parser_only ]-----------------------------
//  Used for debugging the parser callbacks
//-----------------------------------------------------------------------------

void print_parser_only(const char* name, const http_parser *p) 
{
	printf("\n%s:\n", name);
	printf("  http_parser: %p\n", p);
}

//---------------------------[ print_all ]-------------------------------------
//  Used for debugging the parser callbacks
//-----------------------------------------------------------------------------

void print_all(const char* name, const http_parser *p, const char *buf, size_t len) 
{
	print_parser_only(name, p);
	printf("  buf: %p\n", buf);
	printf("  len: %zu\n", len);
	if (buf != 0 && len > 0)
		hexDump(buf, len, "Memory");
}
