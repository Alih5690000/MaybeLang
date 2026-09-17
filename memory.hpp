#include <cstddef>

#include <iostream>
#include <stdexcept>

#ifdef DEBUG

#define LOG(x) std::cerr<<x<<std::endl

#else

#define LOG

#endif

template <typename T>

class Block;

template <typename T>

class Pointer{

public:

Block<T>* _block=nullptr;

Pointer(){}

Pointer(Block<T>* b){

LOG("Pointer constructor");

_block=b;

if (_block)

_block->_refcount++;

}

Pointer(const Pointer<T>& o){

LOG("Pointer copy constructor");

_block=o._block;

if (_block)

_block->_refcount++;

}

Pointer<T>& operator=(const Pointer<T>& o){

if (this==&o)

return *this;

if (_block && --_block->_refcount==0){

    delete _block;

    _block = nullptr;

}

_block=o._block;

if (_block)

_block->_refcount++;

return *this;

}

Pointer(Pointer<T>&& o) noexcept{

_block=o._block;

o._block=nullptr;

}

Pointer<T>& operator=(Pointer<T>&& o) noexcept{

if (this==&o)

return *this;

if (_block && --_block->_refcount==0){

    delete _block;

    _block = nullptr;

}

_block=o._block;

o._block=nullptr;

return *this;

}

T* get(){

if (_block)

return _block->_data;

else

return nullptr;

}

T* operator->(){

if (!_block)

throw std::runtime_error("Null pointer dereference");

return _block->_data;

}

~Pointer(){

LOG("Pointer destructor");

if (_block){

    _block->_refcount--;

    if (_block->_refcount==0){

        delete _block;

        _block = nullptr;

    }

}

}

};

template <typename T>

class Block{

public:

T* _data;

size_t _refcount=0;

Block(T* _ptr):_data(_ptr){

LOG("Block constructor");

}

T* getData(){

return _data;

}

~Block(){

LOG("Block destructor");

delete _data;

}

};

template <typename T>

Pointer<T> MakePtr(T* a){

if (!a)

return Pointer<T>();

Pointer<T> p(new Block<T>(a));

return p;

}