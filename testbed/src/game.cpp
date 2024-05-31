#include "game.hpp"

#include <core/logger.hpp>

bool testgame::initialize(){
    KDEBUG("game_initialize() called!");
    return true;
}

bool testgame::update(f32 delta_time){
    return true;
}

bool testgame::render(f32 delta_time){
    return true;
}

void testgame::on_resize(u32 width, u32 height){

}