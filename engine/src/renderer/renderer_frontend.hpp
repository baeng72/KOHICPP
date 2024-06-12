#include "renderer_types.inl"

class renderer{
    
    renderer_backend* backend{nullptr};
    static renderer*state_ptr;
    bool begin_frame(f32 delta_time);
    bool end_frame(f32 delta_time);
    
    public:
    bool initialize(ccharp application_name);
    void shutdown();
    void on_resized(u16 width, u16 height);
    bool draw_frame(renderer_packet*packet);
};