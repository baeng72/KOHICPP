#pragma once

#include "defines.hpp"
#include "containers/darray.hpp"


struct event_context{
    //128 bytes
    union{       
        i64 i64[2];
        u64 u64[2];
        f64 f64[2];

        i32 i32[4];
        u32 u32[4];
        f32 f32[4];

        i16 i16[8];
        u16 u16[8];

        i8 i8[16];
        u8 u8[16];

        char c[16]; 
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
    static event_system*instance();
    bool initialize();
    void shutdown();
    bool register_event(u16 code, void* listener, PFN_on_event on_event);
    bool unregister_event(u16 code, void*listener, PFN_on_event on_event);
    bool fire(u16 code, void*sender, event_context&context);
};

enum system_event_code{
    //shuts the application down
    EVENT_CODE_APPLICATION_QUIT = 0x01,

    //keyboard pressed, key_code = data.data.u16[0]
    EVENT_CODE_KEY_PRESSED = 0x02,

    //Keyboard release, key_code = data.data.u16[0]
    EVENT_CODE_KEY_RELEASED = 0x03,

    //mouse button pressed button = data.data.u16[0]
    EVENT_CODE_BUTTON_PRESSED = 0x04,
    
    //mouse button released button = data.data.u16[0]
    EVENT_CODE_BUTTON_RELEASED = 0x05,

    //mouse moved, x = data.data.u16[0], y = data.data.u16[1]
    EVENT_CODE_MOUSE_MOVED = 0x06,

    //Mouse wheeel, z_delta = data.data.u8[0]
    EVENT_CODE_MOUSE_WHEEL = 0x07,

    //Resized/resolution change from OS, width = data.data.u16[0], height = data.data.u16[1]
    EVENT_CODE_RESIZED = 0x08,

    EVENT_CODE_DEBUG0 = 0x10,
    EVENT_CODE_DEBUG1 = 0x11,
    EVENT_CODE_DEBUG2 = 0x12,
    EVENT_CODE_DEBUG3 = 0x13,
    EVENT_CODE_DEBUG4 = 0x14,

    MAX_EVENT_CODE = 0xFF

};

#define event_initialize() (event_system::instance()->initialize())
#define event_shutdown() (event_system::instance()->shutdown())
#define event_fire(c,s,ev) (event_system::instance()->fire((c),(s),(ev)))