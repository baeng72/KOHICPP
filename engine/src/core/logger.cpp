#include "logger.hpp"
#include "asserts.hpp"
#include "platform/platform.hpp"

#include <cstdio>

#include <cstring>
#include <cstdarg>


static constexpr u32 k_log_size=32000;
static char log_buffer[k_log_size];

static logging_system* state_ptr{nullptr};

bool logging_system::initialize(){
    state_ptr = this;
    initialized = true;
    return true;
}

void logging_system::shutdown(){
    state_ptr=nullptr;
}

void logging_system::log_output(log_level level, ccharp message, ...){
    ccharp level_strings[] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
    bool is_error = level < LOG_LEVEL_WARN;

    strcpy_s(log_buffer,k_log_size,level_strings[level]);
    int len = (int)strlen(log_buffer);
    va_list args;
    va_start( args, message );
#if defined(_MSC_VER)
    len += vsnprintf_s( log_buffer+len,k_log_size, k_log_size, message, args );
#else
    len += vsnprintf( log_buffer+len, k_log_size, format, args );
#endif
    va_end(args);
    log_buffer[len] = '\n';
    log_buffer[len+1] = 0;
    if(is_error){
        platform_console_write_error(log_buffer,level);
    }else{
        platform_console_write(log_buffer, level);
    }
   
};


void report_assertion_failure(ccharp expression, ccharp message, ccharp file, ccharp function, i32 line){
    logging_system::log_output(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: %s, in file: %s, function %s, line %d\n", expression, message, file, function, line);
}