#include "game.hpp"

#include <core/logger.hpp>

bool testgame::initialize(){
    KDEBUG("game_initialize() called!");
    return true;
}

bool testgame::update(f32 delta_time){
    static u64 alloc_count = 0;
    u64 prev_alloc_count = alloc_count;
    alloc_count = get_memory_alloc_count();
    if(input_is_key_up(KEY_M) && input_was_key_down(KEY_M)){
        KDEBUG("Allocations: %llu (%llu this frame)",alloc_count, alloc_count-prev_alloc_count);
    }
    return true;
}

bool testgame::render(f32 delta_time){
    return true;
}

void testgame::on_resize(u32 width, u32 height){

}