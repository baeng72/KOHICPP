#pragma once

#include "defines.hpp"

#include "core/input.hpp"

struct application_config{
    i16 start_pos_x;
    i16 start_pos_y;
    i16 start_width;
    i16 start_height;
    char * name;
};

struct game;

class KAPI application{        
    input input;
    public:
    bool create(game*game_inst);
    bool run();
};

