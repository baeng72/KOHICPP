#include <core/logger.hpp>
#include <core/asserts.hpp>
#include <platform/platform.hpp>

int main(){
    KFATAL("A test message: %f",3.14f);
    KERROR("A test message: %f", 3.14f);
    KWARN("A test message: %f", 3.14f)
    KINFO("A test message: %f",3.14f);
    KDEBUG("A test message: %f",3.14f);
    KTRACE("A test message: %f", 3.14f);
    platform platform;
    if(platform.startup("Kohi Engine Testbed",100,100,1280,720)){
        while(true){
            platform.pump_messages();
        }
        platform.shutdown();
    }
    return 0;
}