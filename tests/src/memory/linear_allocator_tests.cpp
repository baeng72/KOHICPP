#include "linear_allocator_tests.hpp"
#include "../test_manager.hpp"
#include "../expect.hpp"

#include <defines.hpp>

#include <memory/linear_allocator.hpp>

u8 linear_allocator_should_create_and_destroy(){
    linear_allocator alloc;
    alloc.create(sizeof(u64),0);
    expect_should_not_be(0, alloc.memory);
    expect_should_be(sizeof(u64), alloc.total_size);
    expect_should_be(0, alloc.allocated);

    alloc.destroy();

    expect_should_be(0, alloc.memory);
    expect_should_be(0, alloc.total_size);
    expect_should_be(0, alloc.allocated);
    return true;
}

u8 linear_allocator_single_allocation_all_space(){
    linear_allocator alloc;
    alloc.create(sizeof(u64),0);

    //Single allocation
    void * block = alloc.allocate(sizeof(u64));

    //Validate it
    expect_should_not_be(0, block);
    expect_should_be(sizeof(u64),alloc.allocated);

    alloc.destroy();

    return true;
}

u8 linear_allocator_multi_allocation_all_space(){
    u64 max_allocs = 1024;
    linear_allocator alloc;
    alloc.create(sizeof(u64) * max_allocs, 0);

    //Multiple allocations - full.
    void * block;
    for(u64 i=0; i< max_allocs; ++i){
        block = alloc.allocate(sizeof(u64));
        //Validate it
        expect_should_not_be(0, block);
        expect_should_be(sizeof(u64) * (i+1), alloc.allocated);
    }

    alloc.destroy();

    return true;
}

u8 linear_allocator_multi_allocation_over_allocate(){
    u64 max_allocs = 3;
    linear_allocator alloc;
    alloc.create(sizeof(u64) * max_allocs,0);

    //multiple allocations - full.
    void * block;
    for(u64 i = 0; i < max_allocs; ++i){
        block = alloc.allocate(sizeof(u64));
        //Validate it
        expect_should_not_be(0,block);
        expect_should_be(sizeof(u64) * (i+1), alloc.allocated);
    }

    KDEBUG("Note: The following error is intentionally caused by this test.");

    //Ask for one more allocation. Should error and return 0.
    block = alloc.allocate(sizeof(u64));
    //Validate it - allocated shold be unchanged.
    expect_should_be(0, block);
    expect_should_be(sizeof(u64) * (max_allocs),alloc.allocated);

    alloc.destroy();
    return true;
}

u8 linear_allocator_multi_allocation_all_space_then_free(){
    u64 max_allocs = 1024;
    linear_allocator alloc;
    alloc.create(sizeof(u64) * max_allocs,0);

    //Multiple allocations - full
    void * block;
    for(u64 i =0 ;  i < max_allocs; ++i){
        block = alloc.allocate(sizeof(u64));
        //Validate it
        expect_should_not_be(0, block);
        expect_should_be(sizeof(u64) * (i + 1), alloc.allocated);
    }

    //Validate that pointer is reset.
    alloc.free_all();
    expect_should_be(0, alloc.allocated);

    alloc.destroy();

    return true;
}

void linear_allocator_register_tests(test_manager&manager){
    manager.register_test(linear_allocator_should_create_and_destroy,"Linear allocator should create and destroy.");
    manager.register_test(linear_allocator_single_allocation_all_space, "Linear allocator single alloc for all space");
    manager.register_test(linear_allocator_multi_allocation_all_space, "Linear allocator multi alloc for all space");
    manager.register_test(linear_allocator_multi_allocation_over_allocate, "Linear allocator try over allocate");
    manager.register_test(linear_allocator_multi_allocation_all_space_then_free, "Linear allocator allocated should be 0 after free_all");
}