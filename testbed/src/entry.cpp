#include "game.hpp"

#include <entry.hpp>

game*create_game(){
    application_config app_config;
    app_config.start_pos_x = 100;
    app_config.start_pos_y = 100;
    app_config.start_width = 1280;
    app_config.start_height = 800;
    app_config.name = "Kohi Engine Testbed";
    return new testgame(app_config);

}
