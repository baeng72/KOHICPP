#pragma once

#include <defines.hpp>
#include <game_types.hpp>
#include <core/kmemory.hpp>

struct testgame : public game{
    struct game_state{
    f32 delta_time;
    }*state;
    testgame(application_config ac){
        app_config=ac;
        state = (game_state*)kallocate(sizeof(game_state),MEMORY_TAG_GAME);
    }
    virtual bool initialize()override;
    virtual bool update(f32 delta_time)override;
    virtual bool render(f32 delta_time)override;
    void on_resize(u32 width, u32 height)override;
};