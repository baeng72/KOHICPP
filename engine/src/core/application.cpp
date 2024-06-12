#include "application.hpp"
#include "game_types.hpp"

#include "logger.hpp"

#include "platform/platform.hpp"
#include "core/kmemory.hpp"
#include "core/event.hpp"
#include "core/clock.hpp"

#include "memory/linear_allocator.hpp"

#include "renderer/renderer_frontend.hpp"


struct application_state{
    game*game_inst;
    bool is_running;
    bool is_suspended;
    
    
    i16 width;
    i16 height;
    clock clock;
    f64 last_time;
    linear_allocator systems_allocator;

    platform_system *pplatform;

    renderer* prenderer;

    memory_system*pmemory;

    logging_system*plogging;

    input_system* pinput;

    event_system* pevent;


};

static application_state * app_state;


//static application_state app_state;


bool application::create(game*game_inst){
    if(game_inst->application_state){
        KERROR("application::create called more than once.");
        return false;
    }

    game_inst->application_state = (application_state*)kallocate(sizeof(application_state),MEMORY_TAG_APPLICATION);
    app_state = game_inst->application_state;
    app_state->game_inst = game_inst;
    app_state->is_running = false;
    app_state->is_suspended = false;

    u64 systems_allocator_total_size = 64 * 1024 * 1024;//64 MiB
    app_state->systems_allocator.create(systems_allocator_total_size,nullptr);

    app_state->pevent = (event_system*)app_state->systems_allocator.allocate(sizeof(event_system));
    app_state->pevent = new(app_state->pevent) event_system();//need to run constructor of darray here
    app_state->pevent->initialize();
    
    //app_state->pmemory = new(pmem) memory_system;//use placement new to call constructor? probably not needed
    app_state->pmemory = (memory_system*)app_state->systems_allocator.allocate(sizeof(memory_system));
    app_state->pmemory = new(app_state->pmemory) memory_system();//just in case there's something to be constructed
    app_state->pmemory->initialize();

    app_state->plogging = (logging_system*)app_state->systems_allocator.allocate(sizeof(logging_system));
    app_state->plogging = new(app_state->plogging) logging_system();//just in case there's something to be constructed
    app_state->plogging->initialize();
    //app_state->plogging = new(pmem) Log;
    //initialize subsystems
    
    
    app_state->pinput = (input_system*)app_state->systems_allocator.allocate(sizeof(input_system));
    app_state->pinput = new(app_state->pinput) input_system();
    app_state->pinput->initialize();

    app_state->is_running=true;
    app_state->is_suspended=false;
    
    
    event_register(EVENT_CODE_APPLICATION_QUIT, 0, application::on_event);
    event_register(EVENT_CODE_KEY_PRESSED, 0, application::on_key);
    event_register(EVENT_CODE_KEY_RELEASED, 0, application::on_key);
    event_register(EVENT_CODE_RESIZED, 0 , application::on_resized);

    app_state->pplatform = (platform_system*)app_state->systems_allocator.allocate(sizeof(platform_system));
    app_state->pplatform = new(app_state->pplatform) platform_system();

    if(!app_state->pplatform->startup(
        game_inst->app_config.name,
        game_inst->app_config.start_pos_x,
        game_inst->app_config.start_pos_y,
        game_inst->app_config.start_width,
        game_inst->app_config.start_height)){
        return false;
    }

    app_state->prenderer = (renderer*)app_state->systems_allocator.allocate(sizeof(renderer));
    app_state->prenderer = new(app_state->prenderer) renderer();
    if(!app_state->prenderer->initialize(game_inst->app_config.name)){
        KFATAL("Failed to initialize renderer. Aborting application");
        return false;
    }

    //initialize the game
    if(!app_state->game_inst->initialize()){
        KFATAL("Game failed to initialize.");
        return false;
    }

    app_state->game_inst->on_resize(app_state->width, app_state->height);

    

    return true;
}

bool application::run(){
    application_state & state = *app_state;
    state.clock.start();
    state.clock.update();
    state.last_time = state.clock.elapsed;
    f64 running_time = 0;
    u8 frame_count = 0;
    f64 target_frame_seconds = 1.f/60;
    KINFO(get_memory_usage_str());
    while(state.is_running){
        if(!state.pplatform->pump_messages()){
            state.is_running = false;
        }

        if(!state.is_suspended){
            //update clock and get delta
            state.clock.update();
            f64 current_time = state.clock.elapsed;
            f64 delta = (current_time - state.last_time);
            f64 frame_start_time = platform_get_absolute_time();

            if(!state.game_inst->update((f32)delta)){
                KFATAL("Game update failed, shutting down.");
                state.is_running = false;
                break;
            }

            //call the game's render routine
            if(!state.game_inst->render((f32)delta)){
                KFATAL("Game render failed, shutting down.");
                state.is_running = false;
                break;
            }

            renderer_packet packet;
            packet.delta_time = (f32)delta;
            state.prenderer->draw_frame(&packet);

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

            input_update(delta);

            state.last_time = current_time;
        }
    }
    state.is_running=false;

    event_unregister(EVENT_CODE_APPLICATION_QUIT,0,application::on_event);
    event_unregister(EVENT_CODE_KEY_PRESSED, 0, application::on_key);
    event_unregister(EVENT_CODE_KEY_RELEASED, 0, application::on_key);
    
    state.pinput->shutdown();
    
    state.prenderer->shutdown();
    state.pplatform->shutdown();

    state.pmemory->shutdown();
    state.plogging->shutdown();
    state.pevent->shutdown();

    return true;
}

void application::get_framebuffer_size(u32 *width, u32*height){
    application_state & state = *app_state;
    *width = state.width;
    *height = state.height;
}

bool application::on_event(u16 code, void* sender, void*listener_inst, event_context&context){
    application_state & state = *app_state;
    switch(code){
        case EVENT_CODE_APPLICATION_QUIT:{
            KINFO("EVENT_CODE_APPLICATION_QUIT received, shutting down.");
            state.is_running = false;
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
    application_state & state = *app_state;
    if(code == EVENT_CODE_RESIZED){
        u16 width = context.u16[0];
        u16 height = context.u16[1];

        //Check if different. If so, trigger a resize event.
        if(width != state.width || height != state.height){
            state.width = width;
            state.height = height;

            KDEBUG("Window resize: %i, %i",width, height);

            //Handle minimization
            if(width == 0 || height == 0){
                KINFO("Window minimized, suspending application.");
                state.is_suspended = true;
                return true;
            }else{
                if(state.is_suspended){
                    KINFO("Window restored, resuming application.");
                    state.is_suspended = false;
                }
                state.game_inst->on_resize(width,height);
                state.prenderer->on_resized(width,height);
            }
        }
    }
    return false;//allow other listeners to process message
}