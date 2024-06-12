#include "core/input.hpp"
#include "core/event.hpp"
#include "core/kmemory.hpp"
#include "core/logger.hpp"

input_system*input_system::state_ptr{nullptr};

void input_system::initialize(){
    state_ptr=this;
    auto&state = *state_ptr;
    kzero_memory(&state,sizeof(state));
    initialized= true;
    KINFO("Input subsystem initialized.\n");
}

void input_system::shutdown(){
    state_ptr=nullptr;
}

void input_system::update(f64 delta_time){
    if(state_ptr==nullptr)
        return;
    auto&state = *state_ptr;
    //Copy current states to previous states.
    kcopy_memory(&state.keyboard_previous,&state.keyboard_current,sizeof(keyboard_state));
    kcopy_memory(&state.mouse_previous,&state.mouse_current,sizeof(mouse_state));
}

void input_system::process_key(keys key, bool pressed){
    if(state_ptr==nullptr)
        return;
    auto&state = *state_ptr;
        if (key == KEY_LEFT_ALT) {
        KINFO("Left alt pressed.");
    } else if (key == KEY_RIGHT_ALT) {
        KINFO("Right alt pressed.");
    }

    if (key == KEY_LEFT_CONTROL) {
        KINFO("Left ctrl pressed.");
    } else if (key == KEY_RIGHT_CONTROL) {
        KINFO("Right ctrl pressed.");
    }

    if (key == KEY_LEFT_SHIFT) {
        KINFO("Left shift pressed.");
    } else if (key == KEY_RIGHT_SHIFT) {
        KINFO("Right shift pressed.");
    }
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

void input_system::process_button(buttons button, bool pressed){
    if(state_ptr==nullptr)
        return;
    auto&state = *state_ptr;
    if(state.mouse_current.buttons[button]!=pressed){
        state.mouse_current.buttons[button] = pressed;

        //Fire the event
        event_context context;
        context.u16[0] = button;
        event_fire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED,0,context);
    }
}

void input_system::process_mouse_move(i16 x, i16 y){
    if(state_ptr==nullptr)
        return;
    auto&state = *state_ptr;
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

void input_system::process_mouse_wheel(i8 z_delta){
    if(state_ptr==nullptr)
        return;
    auto&state = *state_ptr;
    //Fire the event
    event_context context;
    context.u8[0] = z_delta;
    event_fire(EVENT_CODE_MOUSE_WHEEL,0,context);
}

bool input_system::is_key_down(keys key){
    if(state_ptr==nullptr)
        return false;
    auto&state = *state_ptr;
    return state.keyboard_current.keys[key] == true;
}

bool input_system::is_key_up(keys key){
    if(state_ptr==nullptr)
        return false;
    auto&state = *state_ptr;
    return state.keyboard_current.keys[key] == false;
}

bool input_system::was_key_down(keys key){
    if(state_ptr==nullptr)
        return false;
    auto&state = *state_ptr;
    return state.keyboard_previous.keys[key] == true;
}

bool input_system::was_key_up(keys key){
    if(state_ptr==nullptr)
        return false;
    auto&state = *state_ptr;
    return state.keyboard_previous.keys[key] == false;
}

//mouse input
bool input_system::is_button_down(buttons button){
    if(state_ptr==nullptr)
        return false;
    auto&state = *state_ptr;
    return state.mouse_current.buttons[button]=true;
}

bool input_system::is_button_up(buttons button){
    if(state_ptr==nullptr)
        return false;
    auto&state = *state_ptr;
    return state.mouse_current.buttons[button]=false;
}

bool input_system::was_button_down(buttons button){
    if(state_ptr==nullptr)
        return false;
    auto&state = *state_ptr;
    return state.mouse_previous.buttons[button]=true;
}

bool input_system::was_button_up(buttons button){
    if(state_ptr==nullptr)
        return false;
    auto&state = *state_ptr;
    return state.mouse_previous.buttons[button]=false;
}

void input_system::get_mouse_position(i32*x, i32*y){
    if(state_ptr==nullptr){
        *x = 0;
        *y = 0;
        return;
    }
    auto&state = *state_ptr;
    
    *x = state.mouse_current.x;
    *y = state.mouse_current.y;
}

void input_system::get_mouse_position(i32&x, i32&y){
    if(state_ptr==nullptr){
        x = 0;
        y = 0;
        return;
    }
    auto&state = *state_ptr;
    x = state.mouse_current.x;
    y = state.mouse_current.y;
}

void input_system::get_previous_mouse_position(i32*x, i32*y){
    if(state_ptr==nullptr){
        *x = 0;
        *y = 0;
        return;
    }
    auto&state = *state_ptr;
    *x = state.mouse_previous.x;
    *y = state.mouse_previous.y;
}

void input_system::get_previous_mouse_position(i32&x, i32&y){
    if(state_ptr==nullptr){
        x = 0;
        y = 0;
        return;
    }
    auto&state = *state_ptr;
    x = state.mouse_previous.x;
    y = state.mouse_previous.y;
}
