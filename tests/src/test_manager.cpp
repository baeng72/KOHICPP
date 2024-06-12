#include "test_manager.hpp"

#include <containers/darray.hpp>
#include <core/logger.hpp>
#include <core/kstring.hpp>
#include <core/clock.hpp>

struct test_entry{
    PFN_test func;
    char * desc;
};

static darray<test_entry>  tests;

void test_manager::init(){

}

void test_manager::register_test(PFN_test test, char * desc){
    test_entry e;
    e.func = test;
    e.desc = desc;
    tests.push(e);
}

void test_manager::run_tests(){
    u32 passed = 0;
    u32 failed = 0;
    u32 skipped = 0;

    u32 count = (u32)tests.length();

    clock total_time;
    total_time.start();

    for(u32 i=0 ; i < count; ++i){
        clock test_time;
        test_time.start();
        u8 result = tests[i].func();
        if(result){
            ++passed;
        }else if(result == BYPASS){
            KWARN("[SKIPPED] %s", tests[i].desc);
            ++skipped;
        }else {
            KERROR("[FAILED]: %s", tests[i].desc);
            ++failed;
        }
        char status[20];
        string_format(status, failed ? "**** %d FAILED ***" : "SUCCESS", failed);
        total_time.update();
        KINFO("Executed %d of %d (skipped %d) %s (%.6f sec / %.6f sec total)", i + 1, count, skipped, status, test_time.elapsed, total_time.elapsed);
    }

    total_time.stop();

    KINFO("Results: %d passed, %d failed, %d skipped.", passed, failed, skipped);
}