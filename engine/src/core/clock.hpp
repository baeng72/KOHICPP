

#pragma once

#include "defines.hpp"

struct clock{
    f64 start_time;
    f64 elapsed;    
    void start();
    void stop();
    void update();
};