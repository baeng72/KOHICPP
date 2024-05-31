#include "core/kstring.hpp"
#include "core/kmemory.hpp"

#include <cstring>

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
    return !strcmpi(str0,str1);
}