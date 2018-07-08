#include "utils.h"

//--------------------[ divceil ]----------------------------------------------
//  return the smallest unsigned integer n such that n*b >= a
//
//      (int) ceil(a / b)
//-----------------------------------------------------------------------------

static size_t divceil(size_t a, size_t b)
{
    return (a + b - 1) / b;
}

//-----------------[ roundup ]-------------------------------------------------
//  return the lowest mutiple of b that is greater than or equal to a
//
//  returns b * (int) ceil(a/b)
//-----------------------------------------------------------------------------

size_t roundup(size_t a, size_t b)
{
    return b * divceil(a, b);
}

