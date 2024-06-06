#pragma once

#include "defines.hpp"

#include "core/kmemory.hpp"
#include "core/logger.hpp"

constexpr i32 DARRAY_RESIZE_FACTOR = 2;
constexpr i32 DARRAY_DEFAULT_CAPACITY = 1;

template<typename T> class darray{
    struct darray_state{                
        u64 capacity;
        u64 length;        
        void* memory;
    };
    darray_state*parray{nullptr};
    darray_state * create(u64 capacity){
        darray_state*pstate=nullptr;
        u64 array_size = sizeof(T) * capacity;
        u64 header_size = sizeof(u64) * offsetof(darray_state,memory);
        
        pstate= (darray_state*)kallocate(header_size+array_size, MEMORY_TAG_DARRAY);
        kset_memory(pstate,0,header_size+array_size);
        pstate->length = 0;
        pstate->capacity = capacity;     
        pstate->memory = (u8*)pstate + header_size;
        return pstate;
    }
    void destroy(darray_state*pstate){
        u64 array_size = sizeof(T) * pstate->capacity;
        u64 header_size = sizeof(u64) * offsetof(darray_state,memory);
        kfree(pstate,array_size+header_size,MEMORY_TAG_DARRAY);
    }
public:
    darray(u64 capacity=DARRAY_DEFAULT_CAPACITY){
        parray = create(capacity);
    }

    

    ~darray(){
        destroy(parray);
        parray=nullptr;
        
    }

    void resize(){
        darray_state*ptemp = create(parray->capacity+DARRAY_RESIZE_FACTOR);
        kcopy_memory(ptemp->memory,parray->memory,parray->length*sizeof(T));
        ptemp->length = parray->length;        
        destroy(parray);
        parray = ptemp;
    }

    void push(const T*value_ptr){
        if(parray->length >= parray->capacity){
            resize();
        }
        T* addr = static_cast<T*>(parray->memory);
        addr += length;
        kcopy_memory(addr,value_ptr,sizeof(T));
        parray->length++;
    }
    void push(const T&value){
        if(parray->length >= parray->capacity){
            resize();
        }
        T* addr = static_cast<T*>(parray->memory);
        addr += parray->length;
        kcopy_memory(addr,&value,sizeof(T));
        parray->length++;
    }

    T* pop(){
        T* addr = static_cast<T*>(parray->memory);
        addr += (parray->length-1);
        T ret;
        kcopy_memory(&ret,addr,sizeof(T));
        parray->length--;
        return ret;
    }
    void pop(T&val){
        T* addr = static_cast<T*>(parray->memory);
        addr += (parray->length - 1);
        T ret;
        kcopy_memory(&val,addr,sizeof(T));
        parray->length--;
    }
    void pop(T*val_ptr){
        T* addr = static_cast<T*>(parray->memory);
        addr += (parray->length - 1);
        T ret;
        kcopy_memory(&val_ptr,addr,sizeof(T));
        parray->length--;
    }
    void insert_at(u64 index,const T*value_ptr){
        if(index >= parray->index){
            KERROR("Index outside the bounds of this array! Length: %i, index: %i", parray->length, index)
            return;
        }
        if(parray->length >= parray->capacity){
            resize();
        }
        T* addr = static_cast<T>(parray->memory);
        addr += index;
        if(index != parray->length-1){
            //If not last element in array, move rest outward
            kcopy_memory(&addr[1],&addr[0],sizeof(T)*(parray->length-index));
        }
        kcopy_memory(addr,value_ptr,sizeof(T))        
        parray->length++;
    }
    void insert_at(u64 index,const T&value){
        if(index >= parray->index){
            KERROR("Index outside the bounds of this array! Length: %i, index: %i", parray->length, index)
            return;
        }
       if(parray->length >= parray->capacity){
            resize();
        }
        T* addr = static_cast<T*>(parray->memory);
        addr += index;
        if(index != parray->length-1){
            //If not last element in array, move rest outward
            kcopy_memory(&addr[1],&addr[0],sizeof(T)*(parray->length-index));
        }
        kcopy_memory(addr,&value,sizeof(T));
        parray->length++;
    }

    T* pop_at(u64 index){
        if(index >= parray->length){
            KERROR("Index outside the bounds of this array! Length %i, index: %i",parray->length,index);
            return;
        }
        T* addr = static_cast<T*>(parray->memory);
        addr+=index;
        T ret;
        kcopy_memory(&ret,addr,sizeof(T));
        if(index != length - 1){
            kcopy_memory(&addr[0],&addr[1],sizeof(T)*(parray->length-index));
        }

        parray->length--;
        return ret;
    }
    void pop_at(u64 index, T&val){
        if(index >= parray->length){
            KERROR("Index outside the bounds of this array! Length %i, index: %i",parray->length,index);
            return;
        }
        T* addr = static_cast<T*>(parray->memory);
        addr+=index;
        
        kcopy_memory(&val,addr,sizeof(T));
        if(index != parray->length - 1){
            kcopy_memory(&addr[0],&addr[1],sizeof(T)*(parray->length-index));
        }

        parray->length--;
    }
    void pop_at(u64 index, T*val_ptr){
        if(index >= parray->length){
            KERROR("Index outside the bounds of this array! Length %i, index: %i",parray->length,index);
            return;
        }
        T* addr = static_cast<T*>(parray->memory);
        addr+=index;
        kcopy_memory(&val_ptr,addr,sizeof(T));
        if(index != parray->length - 1){
            kcopy_memory(&addr[0],&addr[1],sizeof(T)*(parray->length-index));
        }

        parray->length--;
    }

    u64 capacity()const{return parray->capacity;}
    u64 length()const{return parray->length;}
    u64 stride()const{return sizeof(T);}
    void set_length(u64 length){parray->length = length;}
    void clear(){parray->length = 0;}

    T& operator[](u64 index){
        T* addr = static_cast<T*>(parray->memory);
        addr += index;
        return (*(addr));
        }
    const T& operator[](u64 index)const{
        const T* addr = static_cast<T*>(parray->memory);
        addr += index;
        return *(addr);
    }
    operator T* () {return static_cast<T*>((void*)parray->memory);}
};