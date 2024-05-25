#pragma once

#include "core/application.hpp"
#include "core/logger.hpp"
#include "game_types.hpp"

extern game* create_game();

int main(void){
    
    game*pgame = create_game();

    if(!pgame){
        KFATAL("Could not create game!");
        return -1;
    }

    application app;
    if(!app.create(pgame)){
        KINFO("Application failed to create!");
        return 1;
    }

    //begin game loop
    if(!app.run()){
        KINFO("Application did not shutdown gracefully.");
        return 2;
    }

    delete pgame;
    return 0;
}