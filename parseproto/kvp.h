#pragma once

typedef struct _start_count_t {
    size_t start;   // starting index
    size_t count;   // count of consecutive things
} start_count_t;

typedef struct _kvp_t {
    start_count_t key;
    start_count_t value;
} kvp_t;

typedef struct _dict_t {
    size_t nAllocated;
    size_t nUsed;
    kvp_t* kvps;
    char* buf;
} dict_t;

void kvp_test();