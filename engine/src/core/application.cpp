#include "application.hpp"
#include "game_types.hpp"

#include "logger.hpp"

#include "platform/platform.hpp"
#include "core/kmemory.hpp"
#include "core/event.hpp"
#include "core/clock.hpp"

#include "renderer/renderer_frontend.hpp"


struct application_state{
    game*game_inst;
    bool is_running;
    bool is_suspended;
    platform platform;
    renderer renderer;
    i16 width;
    i16 height;
    clock clock;
    f64 last_time;
};

bool initialized = false;
static application_state app_state;


bool application::create(game*game_inst){
    if(initialized){
        KERROR("application::create called more than once.");
        return false;
    }

    app_state.game_inst = game_inst;

    //initialize subsystems
    logger_initialize();
    
    input_initialize();

    app_state.is_running=true;
    app_state.is_suspended=false;
    
    if(!event_initialize()){
        KERROR("Event system failed initialization. Application cannot continue.");
        return false;
    }

    event_register(EVENT_CODE_APPLICATION_QUIT, 0, application::on_event);
    event_register(EVENT_CODE_KEY_PRESSED, 0, application::on_key);
    event_register(EVENT_CODE_KEY_RELEASED, 0, application::on_key);
    event_register(EVENT_CODE_RESIZED, 0 , application::on_resized);

    if(!app_state.platform.startup(
        game_inst->app_config.name,
        game_inst->app_config.start_pos_x,
        game_inst->app_config.start_pos_y,
        game_inst->app_config.start_width,
        game_inst->app_config.start_height)){
        return false;
    }

    if(!app_state.renderer.initialize(game_inst->app_config.name,&app_state.platform)){
        KFATAL("Failed to initialize renderer. Aborting application");
        return false;
    }

    //initialize the game
    if(!app_state.game_inst->initialize()){
        KFATAL("Game failed to initialize.");
        return false;
    }

    app_state.game_inst->on_resize(app_state.width, app_state.height);

    initialized=true;

    return true;
}

bool application::run(){
    app_state.clock.start();
    app_state.clock.update();
    app_state.last_time = app_state.clock.elapsed;
    f64 running_time = 0;
    u8 frame_count = 0;
    f64 target_frame_seconds = 1.f/60;
    KINFO(get_memory_usage_str());
    while(app_state.is_running){
        if(!app_state.platform.pump_messages()){
            app_state.is_running = false;
        }

        if(!app_state.is_suspended){
            //update clock and get delta
            app_state.clock.update();
            f64 current_time = app_state.clock.elapsed;
            f64 delta = (current_time - app_state.last_time);
            f64 frame_start_time = platform_get_absolute_time();

            if(!app_state.game_inst->update((f32)delta)){
                KFATAL("Game update failed, shutting down.");
                app_state.is_running = false;
                break;
            }

            //call the game's render routine
            if(!app_state.game_inst->render((f32)delta)){
                KFATAL("Game render failed, shutting down.");
                app_state.is_running = false;
                break;
            }

            renderer_packet packet;
            packet.delta_time = (f32)delta;
            app_state.renderer.draw_frame(&packet);

            //Figure out how long the frame took and, if below
            f64 frame_end_time = platform_get_absolute_time();
            f64 frame_time_elapsed = frame_end_time - frame_start_time;
            running_time += frame_end_time;
            f64 remaining_seconds =target_frame_seconds - frame_time_elapsed;

            if(remaining_seconds>0){
                u64 remaining_ms = (u64)(remaining_seconds*1000);
                //If ther is time left, give it back to OS
                bool limit_frames = false;
                if(remaining_ms > 0 && limit_frames){
                    platform_sleep(remaining_ms-1);
                }
                frame_count++;
            }

            input.update(delta);

            app_state.last_time = current_time;
        }
    }
    app_state.is_running=false;

    event_unregister(EVENT_CODE_APPLICATION_QUIT,0,application::on_event);
    event_unregister(EVENT_CODE_KEY_PRESSED, 0, application::on_key);
    event_unregister(EVENT_CODE_KEY_RELEASED, 0, application::on_key);
    event_shutdown();
    input_shutdown();
    app_state.renderer.shutdown();
    app_state.platform.shutdown();

    return true;
}

void application::get_framebuffer_size(u32 *width, u32*height){
    *width = app_state.width;
    *height = app_state.height;
}

bool application::on_event(u16 code, void* sender, void*listener_inst, event_context&context){
    switch(code){
        case EVENT_CODE_APPLICATION_QUIT:{
            KINFO("EVENT_CODE_APPLICATION_QUIT received, shutting down.");
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
            KDEBUG("Explicit - A key pressed!");            
        }else{
            KDEBUG("'%c' key pressed in window.",key_code);
        }        
    }
    else if(code == EVENT_CODE_KEY_RELEASED){
        u16 key_code = context.u16[0];
        if(key_code == KEY_B){
            KDEBUG("Explicit - B key released!");
        }else{
            KDEBUG("'%c' key released in window.", key_code);
        }
    }
    return false;
}

bool application::on_resized(u16 code, void*sender, void * listener_inst, event_context&context){
    if(code == EVENT_CODE_RESIZED){
        u16 width = context.u16[0];
        u16 height = context.u16[1];

        //Check if different. If so, trigger a resize event.
        if(width != app_state.width || height != app_state.height){
            app_state.width = width;
            app_state.height = height;

            KDEBUG("Window resize: %i, %i",width, height);

            //Handle minimization
            if(width == 0 || height == 0){
                KINFO("Window minimized, suspending application.");
                app_state.is_suspended = true;
                return true;
            }else{
                if(app_state.is_suspended){
                    KINFO("Window restored, resuming application.");
                    app_state.is_suspended = false;
                }
                app_state.game_inst->on_resize(width,height);
                app_state.renderer.on_resized(width,height);
            }
        }
    }
    return false;//allow other listeners to process message
}