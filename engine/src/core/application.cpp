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

bool application_on_event(u16 code, void* sender, void*listener_inst, event_context&context);
bool application_on_key(u16 code, void* sender, void* listener_inst, event_context&context);

bool application::create(game*game_inst){
    if(initialized){
        KERROR("application::create called more than once.\n");
        return false;
    }

    app_state.game_inst = game_inst;

    //initialize subsystems
    logger_initialize();
    
    input_initialize();

    app_state.is_running=true;
    app_state.is_suspended=false;
    
    if(!event_initialize()){
        KERROR("Event system failed initialization. Application cannot continue.\n");
        return false;
    }

    event_register(EVENT_CODE_APPLICATION_QUIT, 0, application::on_event);
    event_register(EVENT_CODE_KEY_PRESSED, 0, application::on_key);
    event_register(EVENT_CODE_KEY_RELEASED, 0, application::on_key);

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

            input.update(0);
        }
    }
    app_state.is_running=false;

    event_unregister(EVENT_CODE_APPLICATION_QUIT,0,application::on_event);
    event_unregister(EVENT_CODE_KEY_PRESSED, 0, application::on_key);
    event_unregister(EVENT_CODE_KEY_RELEASED, 0, application::on_key);
    event_shutdown();
    input_shutdown();
    app_state.platform.shutdown();

    return true;
}



bool application::on_event(u16 code, void* sender, void*listener_inst, event_context&context){
    switch(code){
        case EVENT_CODE_APPLICATION_QUIT:{
            KINFO("EVENT_CODE_APPLICATION_QUIT received, shutting down.\n");
            app_state.is_running = false;
            return true;
        }
    }
    return false;
}

bool application::on_key(u16 code, void* sender, void *listener_inst, event_context& context){
    if(code == EVENT_CODE_KEY_PRESSED){
        u16 key_code = context.u16[0];
        if(key_code == KEY_ESCAPE){
            event_context data{};
            event_fire(EVENT_CODE_APPLICATION_QUIT,0,data);
            //block anything else from processing this.
            return true;
        }
        else if(key_code == KEY_A){
            //Example on checking for a key
            KDEBUG("Explicit - A key pressed!\n");            
        }else{
            KDEBUG("'%c' key pressed in window.]n",key_code);
        }        
    }
    else if(code == EVENT_CODE_KEY_RELEASED){
        u16 key_code = context.u16[0];
        if(key_code == KEY_B){
            KDEBUG("Explicit - B key released!\n");
        }else{
            KDEBUG("'%c' key released in window.\n", key_code);
        }
    }
    return false;
}