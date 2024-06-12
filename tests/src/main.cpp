#include "test_manager.hpp"

#include "memory/linear_allocator_tests.hpp"

#include <core/logger.hpp>

int main(){
    test_manager manager;
    manager.init();
    linear_allocator_register_tests(manager);
    KDEBUG("Starting tests...");
    manager.run_tests();
    
}