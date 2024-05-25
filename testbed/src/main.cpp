#include <core/logger.hpp>
#include <core/asserts.hpp>

int main(){
    KFATAL("A test message: %f",3.14f);
    KERROR("A test message: %f", 3.14f);
    KWARN("A test message: %f", 3.14f)
    KINFO("A test message: %f",3.14f);
    KDEBUG("A test message: %f",3.14f);
    KTRACE("A test message: %f", 3.14f);
    return 0;
}