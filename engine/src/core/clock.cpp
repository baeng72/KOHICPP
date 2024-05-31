#include "clock.hpp"

#include "platform/platform.hpp"

void clock::start(){
    start_time = platform_get_absolute_time();
    elapsed=0.0;
}

void clock::stop(){
    start_time = 0.0;
}

void clock::update(){
    if(start_time != 0.0){
        elapsed = platform_get_absolute_time() - start_time;
    }
}