#pragma once

#include "defines.hpp"


enum buttons {
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_MIDDLE,
    BUTTON_MAX_BUTTONS
};
#if defined(KPLATFORM_GLFW)
enum keys{
    KEY_SPACE  =  32,
    KEY_APOSTROPHE0=         39,  /* ' */
    KEY_COMMA  =            44,  /* , */
    KEY_MINUS              =45,  /* - */
    KEY_PERIOD             =46,  /* . */
    KEY_SLASH              =47,  /* / */
    KEY_0                  =48,
    KEY_1                  =49,
    KEY_2                  =50,
    KEY_3                  =51,
    KEY_4                  =52,
    KEY_5                  =53,
    KEY_6                  =54,
    KEY_7                  =55,
    KEY_8                  =56,
    KEY_9                  =57,
    KEY_SEMICOLON          =59 , /* ; */
    KEY_EQUAL              =61,  /* = */
    KEY_A                  =65,
    KEY_B                  =66,
    KEY_C                  =67,
    KEY_D                  =68,
    KEY_E                  =69,
    KEY_F                  =70,
    KEY_G                  =71,
    KEY_H                  =72,
    KEY_I                  =73,
    KEY_J                  =74,
    KEY_K                  =75,
    KEY_L                  =76,
    KEY_M                  =77,
    KEY_N                  =78,
    KEY_O                  =79,
    KEY_P                  =80,
    KEY_Q                  =81,
    KEY_R                  =82,
    KEY_S                  =83,
    KEY_T                  =84,
    KEY_U                  =85,
    KEY_V                  =86,
    KEY_W                  =87,
    KEY_X                  =88,
    KEY_Y                  =89,
    KEY_Z                  =90,
    KEY_LEFT_BRACKET       =91 , /* [ */
    KEY_BACKSLASH          =92,  /* \ */
    KEY_RIGHT_BRACKET      =93,  /* ] */
    KEY_GRAVE_ACCENT       =96,  /* ` */
    KEY_WORLD_1            =161, /* non-US #1 */
    KEY_WORLD_2            =162, /* non-US #2 */

    /* Function keys */
    KEY_ESCAPE             =256,
    KEY_ENTER              =257,
    KEY_TAB                =258,
    KEY_BACKSPACE          =259,
    KEY_INSERT             =260,
    KEY_DELETE             =261,
    KEY_RIGHT              =262,
    KEY_LEFT               =263,
    KEY_DOWN               =264,
    KEY_UP                 =265,
    KEY_PAGE_UP            =266,
    KEY_PAGE_DOWN          =267,
    KEY_HOME               =268,
    KEY_END                =269,
    KEY_CAPS_LOCK          =280,
    KEY_SCROLL_LOCK        =281,
    KEY_NUM_LOCK           =282,
    KEY_PRINT_SCREEN       =283,
    KEY_PAUSE              =284,
    KEY_F1                 =290,
    KEY_F2                 =291,
    KEY_F3                 =292,
    KEY_F4                 =293,
    KEY_F5                 =294,
    KEY_F6                 =295,
    KEY_F7                 =296,
    KEY_F8                 =297,
    KEY_F9                 =298,
    KEY_F10                =299,
    KEY_F11                =300,
    KEY_F12                =301,
    KEY_F13                =302,
    KEY_F14                =303,
    KEY_F15                =304,
    KEY_F16                =305,
    KEY_F17                =306,
    KEY_F18                =307,
    KEY_F19                =308,
    KEY_F20                =309,
    KEY_F21                =310,
    KEY_F22                =311,
    KEY_F23                =312,
    KEY_F24                =313,
    KEY_F25                =314,
    KEY_KP_0               =320,
    KEY_KP_1               =321,
    KEY_KP_2               =322,
    KEY_KP_3               =323,
    KEY_KP_4               =324,
    KEY_KP_5               =325,
    KEY_KP_6               =326,
    KEY_KP_7               =327,
    KEY_KP_8               =328,
    KEY_KP_9               =329,
    KEY_KP_DECIMAL         =330,
    KEY_KP_DIVIDE          =331,
    KEY_KP_MULTIPLY        =332,
    KEY_KP_SUBTRACT        =333,
    KEY_KP_ADD             =334,
    KEY_KP_ENTER           =335,
    KEY_KP_EQUAL           =336,
    KEY_LEFT_SHIFT         =340,
    KEY_LEFT_CONTROL       =341,
    KEY_LEFT_ALT           =342,
    KEY_LEFT_SUPER         =343,
    KEY_RIGHT_SHIFT        =344,
    KEY_RIGHT_CONTROL      =345,
    KEY_RIGHT_ALT          =346,
    KEY_RIGHT_SUPER        =347,
    KEY_MENU               =348,

    KEY_LAST               =KEY_MENU
};
enum key_mods{
    KEY_MOD_SHIFT           =0x0001,
    KEY_MOD_CONTROL         =0x0002,
    KEY_MOD_ALT             =0x0004,
    KEY_MOD_SUPER           =0x0008,
    KEY_MOD_CAPS_LOCK       =0x0010,
    KEY_MOD_NUM_LOCK        =0x0020
};
#else

enum keys{
    #define DEFINE_KEY(name, code) KEY_##name = code
    DEFINE_KEY(BACKSPACE, 0x08),
    DEFINE_KEY(ENTER, 0x0D),
    DEFINE_KEY(TAB, 0x09),
    DEFINE_KEY(SHIFT, 0x10),
    DEFINE_KEY(CONTROL, 0x11),

    DEFINE_KEY(PAUSE, 0x13),
    DEFINE_KEY(CAPITAL, 0x14),

    DEFINE_KEY(ESCAPE, 0x1B),

    DEFINE_KEY(CONVERT, 0x1C),
    DEFINE_KEY(NONCONVERT, 0x1D),
    DEFINE_KEY(ACCEPT, 0x1E),
    DEFINE_KEY(MODECHANGE, 0x1F),

    DEFINE_KEY(SPACE, 0x20),
    DEFINE_KEY(PRIOR, 0x21),
    DEFINE_KEY(NEXT, 0x22),
    DEFINE_KEY(END, 0x23),
    DEFINE_KEY(HOME, 0x24),
    DEFINE_KEY(LEFT, 0x25),
    DEFINE_KEY(UP, 0x26),
    DEFINE_KEY(RIGHT, 0x27),
    DEFINE_KEY(DOWN, 0x28),
    DEFINE_KEY(SELECT, 0x29),
    DEFINE_KEY(PRINT, 0x2A),
    DEFINE_KEY(EXECUTE, 0x2B),
    DEFINE_KEY(SNAPSHOT, 0x2C),
    DEFINE_KEY(INSERT, 0x2D),
    DEFINE_KEY(DELETE, 0x2E),
    DEFINE_KEY(HELP, 0x2F),

    DEFINE_KEY(A, 0x41),
    DEFINE_KEY(B, 0x42),
    DEFINE_KEY(C, 0x43),
    DEFINE_KEY(D, 0x44),
    DEFINE_KEY(E, 0x45),
    DEFINE_KEY(F, 0x46),
    DEFINE_KEY(G, 0x47),
    DEFINE_KEY(H, 0x48),
    DEFINE_KEY(I, 0x49),
    DEFINE_KEY(J, 0x4A),
    DEFINE_KEY(K, 0x4B),
    DEFINE_KEY(L, 0x4C),
    DEFINE_KEY(M, 0x4D),
    DEFINE_KEY(N, 0x4E),
    DEFINE_KEY(O, 0x4F),
    DEFINE_KEY(P, 0x50),
    DEFINE_KEY(Q, 0x51),
    DEFINE_KEY(R, 0x52),
    DEFINE_KEY(S, 0x53),
    DEFINE_KEY(T, 0x54),
    DEFINE_KEY(U, 0x55),
    DEFINE_KEY(V, 0x56),
    DEFINE_KEY(W, 0x57),
    DEFINE_KEY(X, 0x58),
    DEFINE_KEY(Y, 0x59),
    DEFINE_KEY(Z, 0x5A),

    DEFINE_KEY(LWIN, 0x5B),
    DEFINE_KEY(RWIN, 0x5C),
    DEFINE_KEY(APPS, 0x5D),

    DEFINE_KEY(SLEEP, 0x5F),

    DEFINE_KEY(NUMPAD0, 0x60),
    DEFINE_KEY(NUMPAD1, 0x61),
    DEFINE_KEY(NUMPAD2, 0x62),
    DEFINE_KEY(NUMPAD3, 0x63),
    DEFINE_KEY(NUMPAD4, 0x64),
    DEFINE_KEY(NUMPAD5, 0x65),
    DEFINE_KEY(NUMPAD6, 0x66),
    DEFINE_KEY(NUMPAD7, 0x67),
    DEFINE_KEY(NUMPAD8, 0x68),
    DEFINE_KEY(NUMPAD9, 0x69),
    DEFINE_KEY(MULTIPLY, 0x6A),
    DEFINE_KEY(ADD, 0x6B),
    DEFINE_KEY(SEPARATOR, 0x6C),
    DEFINE_KEY(SUBTRACT, 0x6D),
    DEFINE_KEY(DECIMAL, 0x6E),
    DEFINE_KEY(DIVIDE, 0x6F),
    DEFINE_KEY(F1, 0x70),
    DEFINE_KEY(F2, 0x71),
    DEFINE_KEY(F3, 0x72),
    DEFINE_KEY(F4, 0x73),
    DEFINE_KEY(F5, 0x74),
    DEFINE_KEY(F6, 0x75),
    DEFINE_KEY(F7, 0x76),
    DEFINE_KEY(F8, 0x77),
    DEFINE_KEY(F9, 0x78),
    DEFINE_KEY(F10, 0x79),
    DEFINE_KEY(F11, 0x7A),
    DEFINE_KEY(F12, 0x7B),
    DEFINE_KEY(F13, 0x7C),
    DEFINE_KEY(F14, 0x7D),
    DEFINE_KEY(F15, 0x7E),
    DEFINE_KEY(F16, 0x7F),
    DEFINE_KEY(F17, 0x80),
    DEFINE_KEY(F18, 0x81),
    DEFINE_KEY(F19, 0x82),
    DEFINE_KEY(F20, 0x83),
    DEFINE_KEY(F21, 0x84),
    DEFINE_KEY(F22, 0x85),
    DEFINE_KEY(F23, 0x86),
    DEFINE_KEY(F24, 0x87),

    DEFINE_KEY(NUMLOCK, 0x90),
    DEFINE_KEY(SCROLL, 0x91),

    DEFINE_KEY(NUMPAD_EQUAL, 0x92),

    DEFINE_KEY(LSHIFT, 0xA0),
    DEFINE_KEY(RSHIFT, 0xA1),
    DEFINE_KEY(LCONTROL, 0xA2),
    DEFINE_KEY(RCONTROL, 0xA3),
    DEFINE_KEY(LMENU, 0xA4),
    DEFINE_KEY(RMENU, 0xA5),

    DEFINE_KEY(SEMICOLON, 0xBA),
    DEFINE_KEY(PLUS, 0xBB),
    DEFINE_KEY(COMMA, 0xBC),
    DEFINE_KEY(MINUS, 0xBD),
    DEFINE_KEY(PERIOD, 0xBE),
    DEFINE_KEY(SLASH, 0xBF),
    DEFINE_KEY(GRAVE, 0xC0),

    KEYS_MAX_KEYS
};
#endif

class KAPI input{
    struct keyboard_state{
        #if defined (KPLATFORM_GLFW)
        bool keys[KEY_LAST];
        #else
        bool keys[256];
        #endif

    };
    struct mouse_state{
        i16 x;
        i16 y;
        bool buttons[BUTTON_MAX_BUTTONS];
    };
    struct input_state{
        keyboard_state keyboard_current;
        keyboard_state keyboard_previous;
        mouse_state mouse_current;
        mouse_state mouse_previous;
    };
    input_state  state;
    bool initialized{false};
    public:    
    static input*instance();
    void initialize();
    void shutdown();
    void update(f64 delta_time);

    //keyboard input
    bool is_key_down(keys key);
    bool is_key_up(keys key);
    bool was_key_up(keys key);
    bool was_key_down(keys key);

    void process_key(keys key, bool pressed);

    //mouse input
    bool is_button_down(buttons button);
    bool is_button_up(buttons button);
    bool was_button_down(buttons button);
    bool was_button_up(buttons button);
    void get_mouse_position(i32*x, i32*y);
    void get_mouse_position(i32&x, i32&y);
    void get_previous_mouse_position(i32*x,i32*y);
    void get_previous_mouse_position(i32&x,i32&y);

    void process_button(buttons button, bool pressed);
    void process_mouse_move(i16 x, i16 y);
    void process_mouse_wheel(i8 z_delta);
};

#define input_initialize() (input::instance()->initialize())
#define input_shutdown() (input::instance()->shutdown())
#define input_update(delta) (input::instance()->update(delta))
#define input_process_key(k,p) (input::instance()->process_key((k),(p)))
#define input_process_button(b,p)(input::instance()->process_button((b),(p)))
#define input_process_mouse_move(x,y)(input::instance()->process_mouse_move((x),(y)))
#define input_process_mouse_wheel(z)(input::instance()->process_mouse_wheel((z)))
#define input_is_key_up(k)(input::instance()->is_key_up(k))
#define input_is_key_down(k)(input::instance()->is_key_down(k))
#define input_was_key_up(k)(input::instance()->was_key_up(k))
#define input_was_key_down(k)(input::instance()->was_key_down(k))
