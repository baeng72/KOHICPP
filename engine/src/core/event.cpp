#include "event.hpp"

#include "core/kmemory.hpp"

event_system s_events;

event_system* event_system::instance(){
    return &s_events;
}

bool event_system::initialize(){
    if(is_initialized){
        return false;
    }

    is_initialized=false;
    //kzero_memory(&state, sizeof(state));

    is_initialized=true;
    return true;
}

void event_system::shutdown(){
    //Free the event arrays. And objects pointed to should be destroyed on their own.    
}

bool event_system::register_event(u16 code, void*listener, PFN_on_event on_event){
    if(!is_initialized){
        return false;
    }

    u64 registered_count = state.registered[code].events.length();
    for(u64 i=0; i < registered_count; ++i){
        if(state.registered[code].events[i].listener == listener){
            //TODO: warn
            return false;
        }
    }
    //If at this point, no duplicate was found. Proceed with registration.
    registered_event event;
    event.listener = listener;
    event.callback = on_event;
    state.registered[code].events.push(event);
    return true;
}

bool event_system::unregister_event(u16 code, void* listener, PFN_on_event on_event){
    if(!is_initialized){
        return false;
    }

    u64 registered_count = state.registered[code].events.length();
    //On nothing is registered for the code, boot out.
    if(registered_count==0){
        return false;
    }

    for(u64 i=0; i < registered_count; ++i){
        registered_event e = state.registered[code].events[i];
        if(e.listener == listener && e.callback == on_event){
            //Found one, remove it
            registered_event popped_event;
            state.registered[code].events.pop_at(i,popped_event);
        }
    }
    //Not found
    return false;    
}

bool event_system::fire(u16 code, void* sender, event_context& context){
    if(!is_initialized)
        return false;
    
    u64 registered_count = state.registered[code].events.length();
    //If nothing is registered for the code, boot
    if(registered_count == 0){
        return false;
    }

    for(u64 i=0; i < registered_count; ++i){
        registered_event e = state.registered[code].events[i];
        if(e.callback(code, sender, e.listener, context)){
            //Message has been handled, do not send to other listeners
            return true;
        }
    }
    //not found
    return false;
}