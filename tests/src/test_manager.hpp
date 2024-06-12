#pragma once

#include <defines.hpp>

#define BYPASS 2

typedef u8 (*PFN_test)();

class test_manager{
    public:
    void init();
    void register_test(PFN_test, char * desc);
    void run_tests();
};