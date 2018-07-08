#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <http_parser.h>
#include "kvp.h"

typedef enum http_method http_method_t;
void hexDump(const void *addr, size_t len);

//-------------------------< http_string_t >-----------------------------------
// stores the location and length of a sting in a buffer
//-----------------------------------------------------------------------------

typedef struct _http_string_t {
    size_t offset;              // offset into buffer where string starts
    size_t len;                 // length of string not including terminator
} http_string_t;

//-------------------------< http_nameval_t >----------------------------------
// stores the name and value associated with an http header
//-----------------------------------------------------------------------------

typedef struct _http_nameval_t {
    http_string_t name;         // name of header
    http_string_t value;        // value of header
} http_nameval_t;

//----------------------< http_namevals_buffer_t >-----------------------------
// stores an array of http_nameval_t structures
//-----------------------------------------------------------------------------

typedef struct _namevals_buffer_t {
    size_t total;           // count of nv structures in array
    size_t used;            // count of nv structures used
    http_nameval_t* anvs;   // pointer to an array nv structures
} namevals_buffer_t;

//----------------------< strings_buffer_t >-----------------------------------
// stores a set of strings 
//-----------------------------------------------------------------------------

typedef struct _strings_buffer_t {
    size_t size;    // count of total bytes in the buffer
    size_t start;   // offset of current string from start of buffer
    size_t len;     // length of current string not including terminator
    char *buf;      // pointer to the start of the buffer
} strings_buffer_t;

//---------------< http_parser_callback_t >------------------------------------
//  Enumerates the parser callback functions
//-----------------------------------------------------------------------------

typedef enum _http_parser_callback_t {
    ON_NONE = 0,
    ON_MESSAGE_BEGIN,
    ON_URL,
    ON_STATUS,
    ON_HEADER_FIELD,
    ON_HEADER_VALUE,
    ON_HEADERS_COMPLETE,
    ON_BODY,
    ON_MESSAGE_COMPLETE,
    ON_CHUNK_HEADER,
    ON_CHUNK_COMPLETE,
} http_parser_callback_t;

//-------------------< http_request_t >----------------------------------------
// contains all state information for http_parser callback sequence
//-----------------------------------------------------------------------------

typedef struct _http_request_t {
    http_parser parser;
    http_parser_callback_t prev;    // previous callback type

    http_method_t method;
    
    unsigned int http_major;
    unsigned int http_minor;
    size_t content_length;
    bool connection_close;

    strings_buffer_t sbuf;          // contains all strings
    size_t buf_inc;

    bool is_url_set;
    http_string_t url;


    namevals_buffer_t nvsbuf;
    size_t current_nv_index;
    size_t nvs_inc;

} http_request_t;

//--------------------< divceil >----------------------------------------------
//  return the smallest unsigned integer n such that n*b >= a
//
//      (int) ceil(a / b)
//-----------------------------------------------------------------------------

static size_t divceil(size_t a, size_t b) 
{
    return (a + b - 1) / b;
}

//-----------------< roundup >-------------------------------------------------
//  return the lowest mutiple of b that is greater than or equal to a
//
//  returns b * (int) ceil(a/b)
//-----------------------------------------------------------------------------

static size_t roundup(size_t a, size_t b) 
{
    return b * divceil(a, b);
}

//-------------------< append_string >-----------------------------------------
//  Appends to the current string in the string buffer. Extra memory is 
//  allocated if needed.
//-----------------------------------------------------------------------------

void append_string(strings_buffer_t *sbuf, const char* str, size_t len, size_t buf_inc)
{
    const size_t teminator_len = 1;
    const size_t size = sbuf->start + sbuf->len + len + teminator_len;
    if (size > sbuf->size) {
        sbuf->size = roundup(size, buf_inc);
        sbuf->buf = realloc(sbuf->buf, sbuf->size);
    }
    memcpy(sbuf->buf + sbuf->start + sbuf->len, str, len);
    sbuf->len = sbuf->len + len;
    sbuf->buf[sbuf->start + sbuf->len] = 0;
}

//-----------------------< add_string >----------------------------------------
//  Add a new string to a strings buffer structure
//-----------------------------------------------------------------------------

void add_string(strings_buffer_t *sbuf, const char* str, size_t len, size_t buf_inc)
{
    if (sbuf->len > 0) {
        sbuf->start += sbuf->len + 1;
        sbuf->len = 0;
    }
    append_string(sbuf, str, len, buf_inc);
}

//-----------------------< pause >---------------------------------------------
//  Prints a message and waits until the user presses ENTER
//-----------------------------------------------------------------------------

void pause(const char* msg)
{
    printf("%s\n", msg);
    char buf[16];
    gets_s(buf, sizeof(buf));
}

//-----------------------< hexDump >------------------------------------------
//  Dumps the contents of a memory buffer
//
//  Arguments:
//
//      addr: starting address of memory to be dumped
//      len: size of the memory to be dumped
//-----------------------------------------------------------------------------

void hexDump(const void *addr, size_t len) 
{
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

//-----------------------< print_parser_only >---------------------------------
//  Used for debugging the parser callbacks
//-----------------------------------------------------------------------------

static void print_parser_only(const char* name, const http_parser *p) {
    printf("\n%s:\n", name);
    printf("  http_parser: %p\n", p);
}

//---------------------< print_all >-------------------------------------------
//  Used for debugging the parser callbacks
//-----------------------------------------------------------------------------

static void print_all(const char* name, const http_parser *p, const char *buf, size_t len) {
    print_parser_only(name, p);
    printf("  buf: %p\n", buf);
    printf("  len: %zu\n", len);
    if (buf != 0 && len > 0)
        hexDump(buf, len);
}

//---------------------< on_message_begin >------------------------------------
// required callback for http_parser
//-----------------------------------------------------------------------------

static int on_message_begin(http_parser* parser) 
{
    print_parser_only("on_message_begin", parser);
    http_request_t* req = (http_request_t*)(parser->data);
    if (req->prev != ON_NONE)
        return 1;
    req->prev = ON_MESSAGE_BEGIN;
    return 0;
}

//--------------------------< on_url >-----------------------------------------
// required callback for http_parser
//-----------------------------------------------------------------------------

static int on_url(http_parser* parser, const char *at, size_t length) 
{
    print_all("on_url", parser, at, length);
    http_request_t* req = (http_request_t*)(parser->data);
    if (req->prev == ON_URL) 
        append_string(&req->sbuf, at, length, req->buf_inc);
    else if (req->prev == ON_MESSAGE_BEGIN)
        add_string(&req->sbuf, at, length, req->buf_inc);
    else
        return 1;
    req->prev = ON_URL;
    return 0;
}

//--------------------------< on_status >--------------------------------------
// required callback for http_parser
//-----------------------------------------------------------------------------

static int on_status(http_parser* parser, const char *at, size_t length)
{
    print_all("on_status", parser, at, length);
    return 0;
}

//--------------------------< on_header_field >--------------------------------
// required callback for http_parser
//-----------------------------------------------------------------------------

static int on_header_field(http_parser* parser, const char *at, size_t length) 
{
    print_all("on_header_field", parser, at, length);
    http_request_t* req = (http_request_t*)(parser->data);
    if (req->prev == ON_URL) {
        if (req->sbuf.len > 0) {
            printf("URL: \"%s\"\n", req->sbuf.buf + req->sbuf.start);
            req->url.offset = req->sbuf.start;
            req->url.len = req->sbuf.len;
            req->is_url_set = true;
        }
        add_string(&req->sbuf, at, length, req->buf_inc);
    }
    else if (req->prev == ON_HEADER_FIELD)
        append_string(&req->sbuf, at, length, req->buf_inc);
    else if (req->prev == ON_HEADER_VALUE) {
        printf("\n");
        //printf("*****************************************************************\n");
        //printf("Key: \"%s\"\n", state->buf);
        //printf("Value: \"%s\"\n", state->buf + state->len_key + 1);
        //printf("*****************************************************************\n");
        //printf("\n");
        //http_headers_add(&(state->req->headers), state->buf, state->len_key);
        //http_headers_set(&(state->req->headers), state->buf, state->buf + state->len_key + 1, state->len_value);
        //state->len_key = 0;
        //state->len_value = 0;
        //state_append_key(state, at, length);
    }
    else {
        req->prev = ON_HEADER_FIELD;
        return 1;
    }
    req->prev = ON_HEADER_FIELD;
    return 0;
}

//-------------------< test_strings >------------------------------------------
// tests adding strings to a strings_buffer_t structure
//-----------------------------------------------------------------------------

void test_strings()
{
    typedef struct _ENTRY {
        const char* msg;
        void(*f)(strings_buffer_t*, const char*, size_t, size_t);
    } ENTRY;

    ENTRY entries[] = {
        { "Hello World", add_string },
        { ". How are you?", append_string },
        { "I am fine, thanks.", add_string },
        { " How about you?", append_string },
        { " Fine, thanks.", add_string }
    };
    strings_buffer_t sbuf = { 0, 0, 0 };
    for (size_t i = 0; i < _countof(entries); i++)
        (*(entries[i].f))(&sbuf, entries[i].msg, strlen(entries[i].msg), 10);
    printf("\n");
    hexDump(sbuf.buf, sbuf.size);
}

//---------------------< find_name_i >-----------------------------------------
// Look in the name-value array to see if the name has already been entered.
// The string comparison is case insensitive. If the name has been found
// the return the index in the name-value array where it is found otherwise
// return SIZE_MAX (name not found).
//-----------------------------------------------------------------------------

static size_t find_name_i(const char* name, size_t len, const namevals_buffer_t* nvs, const char* buf)
{
    printf("\n");
    printf("find_name_i:\n");
    printf("    name: \"%s\"\n", name);
    printf("    len: %zu\n", len);
    printf("    nvs: %p\n", nvs);
    printf("    buf: %p\n", buf);
    size_t ans = SIZE_MAX;
    for (size_t i = 0; i < nvs->used; i++)
    {
        const http_string_t* N = &(nvs->anvs[i].name);
        if (len == N->len && _strnicmp(name, buf + N->offset, len) == 0) {
            ans = i;
            break;
        }
    }
    printf("find_name_i -> %zu\n", ans);
    return ans;
}

void expand_nvs(namevals_buffer_t* nvs, size_t nvs_inc)
{
    if (nvs->used >= nvs->total) {
        size_t new_total = nvs->total + nvs_inc;
        size_t new_size = sizeof(nvs->anvs[0]) * new_total;
        nvs->anvs = (http_nameval_t*)realloc(nvs->anvs, new_size);
        nvs->total = new_total;
    }
}

size_t add_name(const char* name, size_t len, namevals_buffer_t* nvs, strings_buffer_t* sbuf, size_t nvs_inc, size_t buf_inc)
{
    printf("\n");
    printf("add_name:\n");
    printf("    name: \"%s\"\n", name);
    printf("    len: %zu\n", len);
    printf("    nvs: %p\n", nvs);
    printf("    sbuf: %p\n", sbuf);
    printf("    nvs_inc: %zu\n", nvs_inc);
    printf("    buf_inc: %zu\n", buf_inc);
    add_string(sbuf, name, len, buf_inc);
    size_t offset = sbuf->start;
    size_t ans = nvs->used;
    expand_nvs(nvs, nvs_inc);
    http_nameval_t* const nv = &(nvs->anvs[ans]);
    nv->name.len = len;
    nv->name.offset = sbuf->start;
    nv->value.len = 0;
    nv->value.offset = 0;
    nvs->used += 1;
    printf("add_name -> %zu\n", ans);
    return ans;
}

void test_system()
{
    typedef struct _StringBits {
        size_t count;
        const char** bits;
    } StringBits;

    typedef struct _NameVal {
        StringBits NameBits;
        StringBits ValueBits;
    } NameVal;

    const char* bits1[] = { "Do", "g" };
    const char* bits2[] = { "Mam", "al" };
    const char* bits3[] = { "Cat" };
    const char* bits4[] = { "Mamal" };
    const char* bits5[] = { "dog" };
    const char* bits6[] = { "caninie" };


    NameVal NameVals[] = {
        { { _countof(bits1), &bits1 }, { _countof(bits2), &bits2 } },
        { { _countof(bits3), &bits3 }, { _countof(bits4), &bits4 } },
        { { _countof(bits5), &bits5 }, { _countof(bits6), &bits6 } },
    };


    //typedef const char* Pieces [];

    //typedef struct _NameVal {
    //    //Pieces name_pieces;
    //    Pieces name_pieces;
    //    Pieces value_pieces;
    //} NameVal;

    //NameVal NameVals[] = {
    //    {{"Do","g"}, {"Mam", "al"}},
    //    {{"Cat"}, {"Mamal"}},
    //    {{"dog"}, {"canine"}}
    //};

    strings_buffer_t sbuf = { 0, 0, 0 };
    const size_t buf_inc = 10;

    namevals_buffer_t nvsbuf = { 0, 0, 0 };
    const size_t nvs_inc = 2;
    size_t current_nv_index = 0;

    //for (size_t i = 0; i < _countof(NameVals); i++) {
    //    NameVal *p = &(NameVals[i]);
    //    size_t iName = find_name_i(p->name, strlen(p->name), &nvsbuf, sbuf.buf);
    //    if (iName == SIZE_MAX) {
    //        size_t I = add_name(p->name, strlen(p->name), &nvsbuf, &sbuf, nvs_inc, buf_inc);
    //        add_string(&sbuf, p->value, strlen(p->value), buf_inc);
    //        nvsbuf.anvs[I].value.offset = sbuf.start;
    //        nvsbuf.anvs[I].value.len = sbuf.len;
    //    }
    //    else {
    //        // name has been found
    //        printf("iName: %zu\n", iName);
    //    }
    //}
    //printf("\n");
    //hexDump(sbuf.buf, sbuf.size);

}

//--------------------< main >-------------------------------------------------
//  entry point
//-----------------------------------------------------------------------------

int main()
{
    test_system();
    pause("Press ENTER to exit ...");
    return 0;
}
