#include "logger.hpp"
#include "asserts.hpp"

#include <cstdio>

#include <cstring>
#include <cstdarg>

Log s_log;
static constexpr u32 k_log_size=32000;
static char log_buffer[k_log_size];

Log::Log(){

}

Log::~Log(){

}

Log* Log::instance(){
    return &s_log;
}

void Log::log_output(log_level level, ccharp message, ...){
    ccharp level_strings[] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
    
    strcpy_s(log_buffer,k_log_size,level_strings[level]);
    va_list args;
    va_start( args, message );
#if defined(_MSC_VER)
    vsnprintf_s( log_buffer+strlen(log_buffer),k_log_size, k_log_size, message, args );
#else
    vsnprintf( log_buffer, k_log_size, format, args );
#endif
    va_end(args);
    
    

    printf("%s\n",log_buffer);
    
};


