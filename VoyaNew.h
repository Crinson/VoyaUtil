
#pragma once

#include "VoyaUtil/VoyaConfig.h"

// class使用Pool的快捷方式\0

#if VOYA_CLASS_USE_POOL

#define VOYA_POOL_NEW_DECL(_Class) \
void* operator new(size_t _szByte);\
void* operator new(size_t _szByte, const std::nothrow_t& nothrow_value);\
void  operator delete(void* _p, size_t _szByte);

#ifdef MEMORY_POOL_DEBUG

#define VOYA_POOL_NEW_IMPL(_Class) \
void* _Class::operator new(size_t _szByte)\
{\
    return voya::CSTLPoolInstance::instance()->allocate(_szByte, #_Class);\
}\
void* _Class::operator new(size_t _szByte, const std::nothrow_t& nothrow_value)\
{\
    return voya::CSTLPoolInstance::instance()->allocate(_szByte, #_Class);\
}\
void _Class::operator delete(void* _p, size_t _szByte)\
{\
    voya::CSTLPoolInstance::instance()->deallocate(_p, _szByte);\
}

#else

#define VOYA_POOL_NEW_IMPL(_Class) \
void* _Class::operator new(size_t _szByte)\
{\
    return voya::CSTLPoolInstance::instance()->allocate(_szByte);\
}\
void* _Class::operator new(size_t _szByte, const std::nothrow_t& nothrow_value)\
{\
    return voya::CSTLPoolInstance::instance()->allocate(_szByte);\
}\
void _Class::operator delete(void* _p, size_t _szByte)\
{\
    voya::CSTLPoolInstance::instance()->deallocate(_p, _szByte);\
}

#endif




#else //VOYA_CLASS_USE_POOL = 0

#define VOYA_POOL_NEW_DECL(_Class)
#define VOYA_POOL_NEW_IMPL(_Class)

#endif

