#include "core/input.hpp"
#include "core/event.hpp"
#include "core/kmemory.hpp"
#include "core/logger.hpp"

void input::initialize(){
    kzero_memory(&state,sizeof(state));
    initialized= true;
    KINFO("Input subsystem initialized.\n");
}

void input::shutdown(){
    initialized=false;
}

void input::update(f64 delta_time){
    if(!initialized)
        return;

    //Copy current states to previous states.
    kcopy_memory(&state.keyboard_previous,&state.keyboard_current,sizeof(keyboard_state));
    kcopy_memory(&state.mouse_previous,&state.mouse_current,sizeof(mouse_state));
}

void input::process_key(keys key, bool pressed){
    //Only handle this if the state actually changed
    if(state.keyboard_current.keys[key]!= pressed){
        //Update internal state
        state.keyboard_current.keys[key] = pressed;

        //Fire off an event for immediate processing.
        event_context context;
        context.i16[0] = key;
        event_fire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED,0,context);
        
    }
}

void input::process_mouse_move(i16 x, i16 y){
    //Only process if actually different
    if(state.mouse_current.x != x || state.mouse_current.y != y){
        //KDEBUG("Mouse pos: %i, %i!",x,y);

        //Update internal state
        state.mouse_current.x = x;
        state.mouse_current.y = y;

        //Fire the event.
        event_context context;
        context.u16[0] = x;
        context.u16[1] = y;
        event_fire(EVENT_CODE_MOUSE_MOVED, 0, context);
    }
}

void input::process_mouse_wheel(i8 z_delta){
    //Fire the event
    event_context context;
    context.u8[0] = z_delta;
    event_fire(EVENT_CODE_MOUSE_WHEEL,0,context);
}

bool input::is_key_down(keys key){
    if(!initialized)
        return false;
    return state.keyboard_current.keys[key] == true;
}

bool input::is_key_up(keys key){
    if(!initialized)
        return false;
    return state.keyboard_current.keys[key] == false;
}

bool input::was_key_down(keys key){
    if(!initialized)
        return false;
    return state.keyboard_previous.keys[key] == true;
}

bool input::was_key_up(keys key){
    if(!initialized)
        return false;
    return state.keyboard_previous.keys[key] == false;
}

//mouse input
bool input::is_button_down(buttons button){
    if(!initialized)
        return false;
    return state.mouse_current.buttons[button]=true;
}

bool input::is_button_up(buttons button){
    if(!initialized)
        return false;
    return state.mouse_current.buttons[button]=false;
}

bool input::was_button_down(buttons button){
    if(!initialized)
        return false;
    return state.mouse_previous.buttons[button]=true;
}

bool input::was_button_up(buttons button){
    if(!initialized)
        return false;
    return state.mouse_previous.buttons[button]=false;
}

void input::get_mouse_position(i32*x, i32*y){
    if(!initialized){
        *x = *y = 0;
        return;
    }
    *x = state.mouse_current.x;
    *y = state.mouse_current.y;
}

void input::get_mouse_position(i32&x, i32&y){
    if(!initialized){
        x = y = 0;
    }
    x = state.mouse_current.x;
    y = state.mouse_current.y;
}

void input::get_previous_mouse_position(i32*x, i32*y){
    if(!initialized){
        *x = *y = 0;
        return;
    }
    *x = state.mouse_previous.x;
    *y = state.mouse_previous.y;
}

void input::get_previous_mouse_position(i32&x, i32&y){
    if(!initialized){
        x = y = 0;
    }
    x = state.mouse_previous.x;
    y = state.mouse_previous.y;
}
