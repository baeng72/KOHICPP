#include "core/kstring.hpp"
#include "core/kmemory.hpp"

#include <cstring>
#include <cstdio>
#include <cstdarg>

u64 string_length(ccharp str){
    return strlen(str);
}

char* string_duplicate(ccharp str){
    u64 length = strlen(str);
    char * copy = (char*)kallocate(length+1,MEMORY_TAG_STRING);
    kcopy_memory(copy,str,length+1);
    return copy;
}

bool strings_equal(ccharp str0, ccharp str1){
    return !_strcmpi(str0,str1);
}

i32 string_format(char*dest, ccharp format, ...){
    if(dest){
        va_list arg_ptr;
        va_start(arg_ptr, format);
        i32 written = string_format_v(dest, format, arg_ptr);
        va_end(arg_ptr);
        return written;
    }
    return -1;
}

i32 string_format_v(char*dest, ccharp format, va_list va_listp){
    if(dest){
        char buffer[32000];
        i32 written = vsnprintf(buffer,32000,format, va_listp);
        buffer[written] = 0;
        kcopy_memory(dest, buffer, written + 1);
        return written;
    }
    return -1;
}