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
struct event_context;

class KAPI application{        
    input input;
    static bool on_event(u16 code, void*sender, void* listener_inst, event_context& context);
    static bool on_key(u16 code, void* sender, void* listener_inst, event_context& context);
    public:
    bool create(game*game_inst);
    bool run();
};

