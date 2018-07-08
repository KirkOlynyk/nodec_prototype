#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "sbuf.h"
#include "kvp.h"

//---------------------------[ test1 ]-----------------------------------------
//-----------------------------------------------------------------------------

void test1()
{
    void* p = debug_realloc(0, 10);
    debug_free(p);
    p = 0;
}

//---------------------------[ test2 ]-----------------------------------------
//-----------------------------------------------------------------------------

void test2()
{
    sbuf_t sbuf = { 0, 0, 0, 0 };

    size_t const start_key = sbuf_add(&sbuf, "do", 2, 16);
    printf("len: %d\n", sbuf_get_string_length(&sbuf));
    sbuf_append(&sbuf, "g", 1, 16);
    printf("len: %d\n", sbuf_get_string_length(&sbuf));

    size_t const start_value = sbuf_add(&sbuf, "mamal", 5, 16);
    printf("len: %d\n", sbuf_get_string_length(&sbuf));

    hexDump(sbuf.buffer, sbuf.total, "\nsbuf.buffer:");

    printf("\n");
    printf("key:\"%s\"\n", sbuf_get_string(&sbuf, start_key));
    printf("value:\"%s\"\n", sbuf_get_string(&sbuf, start_value));
    printf("\n");

    sbuf_delete(&sbuf);
}

//---------------------------[ test3 ]-----------------------------------------
//-----------------------------------------------------------------------------

void test3()
{
    struct {
        const char* key;
        const char* value;
    } kvps[] =
    {
        { "Dog", "Mamal" },
        { "Shark", "Fish" },
        { "Ant", "Insect"},
    };

    sbuf_t sbuf = { 0, 0, 0, 0 };
    kvpbuf_t kvpbuf = { 0, 0, 0 };

    for (size_t i = 0; i < _countof(kvps); i++) {
        const char* key = kvps[i].key;
        const char* value = kvps[i].value;

        kvp_t kvp = { { 0, 0 }, { 0, 0 } };
        kvp.key.length = strlen(key);
        kvp.value.length = strlen(value);

        kvp.key.start = sbuf_add(&sbuf, key, kvp.key.length, 1024);
        kvp.value.start = sbuf_add(&sbuf, value, kvp.value.length, 1024);

        kvpbuf_add(&kvpbuf, &kvp, 8);
    }

    for (size_t i = 0; i < kvpbuf.used; i++) {
        kvp_t* p_kvp = kvpbuf.buffer + i;
        const char* key = sbuf_get_string(&sbuf, p_kvp->key.start);
        const char* value = sbuf_get_string(&sbuf, p_kvp->value.start);
        printf("\n");
        printf("key:\"%s\" (%u)\n", key, p_kvp->key.length);
        printf("value:\"%s\" (%u)\n", value, p_kvp->value.length);
    }
    printf("\n");
    kvpbuf_delete(&kvpbuf);
    sbuf_delete(&sbuf);
}