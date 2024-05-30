#include "application.hpp"
#include "game_types.hpp"

#include "logger.hpp"

#include "platform/platform.hpp"
#include "core/kmemory.hpp"
#include "core/event.hpp"

struct application_state{
    game*game_inst;
    bool is_running;
    bool is_suspended;
    platform platform;
    i16 width;
    i16 height;
    f64 last_time;
};

bool initialized = false;
static application_state app_state;

bool application::create(game*game_inst){
    if(initialized){
        KERROR("application::create called more than once.\n");
        return false;
    }

    app_state.game_inst = game_inst;

    //initialize subsystems
    Log::instance()->initialize();

    app_state.is_running=true;
    app_state.is_suspended=false;
    
    if(!events.initialize()){
        KERROR("Event system failed initialization. Application cannot continue.\n");
        return false;
    }

    if(!app_state.platform.startup(
        game_inst->app_config.name,
        game_inst->app_config.start_pos_x,
        game_inst->app_config.start_pos_y,
        game_inst->app_config.start_width,
        game_inst->app_config.start_height)){
        return false;
    }

    //initialize the game
    if(!app_state.game_inst->initialize()){
        KFATAL("Game failed to initialize.\n");
        return false;
    }

    app_state.game_inst->on_resize(app_state.width, app_state.height);

    initialized=true;

    return true;
}

bool application::run(){
    KINFO(get_memory_usage_str());
    while(app_state.is_running){
        if(!app_state.platform.pump_messages()){
            app_state.is_running = false;
        }

        if(!app_state.is_suspended){
            if(!app_state.game_inst->update((f32)0.f)){
                KFATAL("Game update failed, shutting down.\n");
                app_state.is_running = false;
                break;
            }

            //call the game's render routine
            if(!app_state.game_inst->render((f32)0.f)){
                KFATAL("Game render failed, shutting down.\n");
                app_state.is_running = false;
                break;
            }
        }
    }
    app_state.is_running=false;

    events.shutdown();

    app_state.platform.shutdown();

    return true;
}



