#pragma once

#include "defines.hpp"
#include "containers/darray.hpp"


struct event_context{
    //128 bytes
    union{
        struct mouse_pos{
            i32 x;
            i32 y;
        }mouse_pos;
    };
};

typedef bool (*PFN_on_event)(u16 code, void* sender, void* listener_inst, event_context&data);

constexpr u32 MAX_MESSAGE_CODES = 16384;

class KAPI event_system{
    struct registered_event{
        void *listener;
        PFN_on_event callback;
    };
    struct event_code_entry{
        darray<registered_event> events;
    };
    struct event_system_state{
        //Lookup table for event codes.
        event_code_entry registered[MAX_MESSAGE_CODES];
    }state;
    bool is_initialized{false};
    public:
    bool initialize();
    void shutdown();
    bool register_event(u16 code, void* listener, PFN_on_event on_event);
    bool unregister_event(u16 code, void*listener, PFN_on_event on_event);
    bool fire(u16 code, void*sender, event_context&context);
};

