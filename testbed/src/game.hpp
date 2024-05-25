#pragma once

#include <defines.hpp>
#include <game_types.hpp>

struct testgame : public game{
    testgame(application_config ac){
        app_config=ac;
    }
    virtual bool initialize()override;
    virtual bool update(f32 delta_time)override;
    virtual bool render(f32 delta_time)override;
    void on_resize(u32 width, u32 height)override;
};