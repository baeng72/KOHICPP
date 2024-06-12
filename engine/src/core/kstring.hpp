#pragma once

#include "defines.hpp"

KAPI u64 string_length(ccharp str);

KAPI char* string_duplicate(ccharp str);

KAPI bool strings_equal(ccharp str0, ccharp str1);

KAPI i32 string_format(char * dest, ccharp format, ...);

KAPI i32 string_format_v(char * dest, ccharp format, va_list va_listp);