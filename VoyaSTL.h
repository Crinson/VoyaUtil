
#pragma once

#include <vector>
#include <list>
#include <map>
#include <unordered_map>

#include "VoyaUtil/VoyaConfig.h"
#include "VoyaUtil/VoyaSingleton.h"
#include "VoyaUtil/VoyaMemory.h"

namespace voya
{

    class CSTLPoolInstance : public CSingleton<CSTLPoolInstance>, public CMemoryPool
    {
    public:
        CSTLPoolInstance()
        {
            setMemoryPoolTag("STL Pool");
        }
    };

#if VOYA_UTIL_USE_SELF_POOL

    template<class _T, typename ... Args>
    _T* newObject(Args ... _args)
    {
        return CSTLPoolInstance::instance()->borrowObject<_T, Args ...>(std::forward<Args>(_args)...);
    }

    template<class _T>
    void deleteObject(_T* _pObject, size_t _ObjectSize = CMemoryPool::AUTO_CLASS_SIZE)
    {
        return CSTLPoolInstance::instance()->returnObject(_pObject, _ObjectSize);
    }

#define GET_CLASS_SIZE(_CLASS) virtual size_t getClassSize() const {return sizeof(_CLASS);}

#else

    template<class _T, typename ... Args>
    _T* newObject(Args ... _args)
    {
        return new _T(std::forward<Args>(_args)...);
    }

    template<class _T>
    void deleteObject(_T* _pObject, size_t _ObjectSize = 0)
    {
        delete _pObject;
    }

#define GET_CLASS_SIZE(_CLASS) virtual size_t getClassSize() const {return sizeof(_CLASS);}

#endif

    template< typename _T >
    class CSTLAllocator : public std::allocator<_T>
    {
    public:
        typedef             size_t                              size_type;
        typedef typename    std::allocator<_T>::pointer         pointer;
        typedef typename    std::allocator<_T>::value_type      value_type;
        typedef typename    std::allocator<_T>::const_pointer   const_pointer;
        typedef typename    std::allocator<_T>::reference       reference;
        typedef typename    std::allocator<_T>::const_reference const_reference;

        template< typename _U >
        struct rebind
        {
            typedef CSTLAllocator<_U> other;
        };

        pointer allocate(size_type _szCount, const void* _pPtr = nullptr)
        {
            return (pointer)voya::CSTLPoolInstance::instance()->allocate(_szCount * sizeof(_T));
        }

        void deallocate(pointer _pPtr, size_type _szCount)
        {
            voya::CSTLPoolInstance::instance()->deallocate(_pPtr, _szCount * sizeof(_T));
        }

        CSTLAllocator() throw()
        {

        }

        CSTLAllocator(const CSTLAllocator& _rhs) throw()
            : std::allocator<_T>(_rhs)
        {

        }

        template< typename _Tp1 >
        CSTLAllocator(const CSTLAllocator<_Tp1>&) throw()
        {

        }

        ~CSTLAllocator() throw()
        {

        }

    };



    //////////////////////////////////////////////////////////////////////////
    //string
    typedef std::basic_string<char, std::char_traits<char>, CSTLAllocator<char>> string;
    


    //////////////////////////////////////////////////////////////////////////
    //vector
    template<typename _T>
    class vector : public std::vector<_T, CSTLAllocator<_T>>
    {
    public:
        vector(const int _iCount) : std::vector<_T, CSTLAllocator<_T>>(_iCount)
        {

        }

        vector() : std::vector<_T, CSTLAllocator<_T>>()
        {

        }
    };


    //////////////////////////////////////////////////////////////////////////
    //list
    template<typename _T>
    class list : public std::list<_T, CSTLAllocator<_T>>
    {
    public:
        list(const int _iCount) : std::list<_T, CSTLAllocator<_T>>(_iCount)
        {

        }

        list() : std::list<_T, CSTLAllocator<_T>>()
        {

        }
    };


    //////////////////////////////////////////////////////////////////////////
    //unorder_map
    template<typename _Key, typename _Value>
    class unordered_map : public std::unordered_map<_Key, _Value, std::hash<_Key>, std::equal_to<_Key>, CSTLAllocator<std::pair<const _Key, _Value>>>
    {
    public:

    };


    //////////////////////////////////////////////////////////////////////////
    //map
    template<typename _Key, typename _Value>
    class map : public std::map<_Key, _Value, std::less<_Key>, CSTLAllocator<std::pair<const _Key, _Value>>>
    {
    public:

    };

}
