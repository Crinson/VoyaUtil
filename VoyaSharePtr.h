
#pragma once

#include <cstddef>
#include "VoyaUtil/VoyaSTL.h"

namespace voya
{

    //////////////////////////////////////////////////////////////////////////
    //指针基类\0
    template<class _T>
    class CPtrBase
    {
    public:
        GET_CLASS_SIZE(CPtrBase);

        virtual void destroy() = 0;
        virtual void destroyThis() = 0;

        CPtrBase(_T* _pPtr)
            : m_pPtr(_pPtr)
        {

        }

        _T* get() const { return m_pPtr; }

    protected:
        _T*     m_pPtr = nullptr;
    };

    //无删除回调函数\0
    template<class _T>
    class CPtr : public CPtrBase<_T>
    {
    public:
        GET_CLASS_SIZE(CPtr);
        virtual void destroy()
        {
            if (nullptr != CPtrBase<_T>::m_pPtr)
            {
                delete CPtrBase<_T>::m_pPtr;
                CPtrBase<_T>::m_pPtr = nullptr;
            }
        }

        virtual void destroyThis()
        {
            deleteObject(this);
        }

        CPtr(_T* _pPtr)
            : CPtrBase<_T>(_pPtr)
        {

        }
    };

    //带删除回调函数\0
    template<class _T, class _Deletor>
    class CPtrDeletor : public CPtrBase<_T>
    {
        using Deletor = _Deletor;

    public:
        GET_CLASS_SIZE(CPtrDeletor);
        virtual void destroy()
        {
            m_deletor(CPtrBase<_T>::m_pPtr);
            CPtrBase<_T>::m_pPtr = nullptr;
        }

        virtual void destroyThis()
        {
            deleteObject(this);
        }

        CPtrDeletor(_T* _pPtr, _Deletor _deletor)
            : CPtrBase<_T>(_pPtr)
            , m_deletor(_deletor)
        {

        }

    private:
        Deletor     m_deletor;

    };


    //////////////////////////////////////////////////////////////////////////
    //计数器\0
    template<class _T>
    class CCounterBase
    {
    public:

        template<class _U>
        CCounterBase(_U* _pPtr)
        {
            m_pPtrBase = newObject<CPtr<_U>>(_pPtr);
        }

        template<class _U, class _Deletor>
        CCounterBase(_U* _pPtr, _Deletor _deletor)
        {
            m_pPtrBase = newObject<CPtrDeletor<_U, _Deletor>>(_pPtr, _deletor);
        }

        ~CCounterBase()
        {
            
        }

        void retain()
        {
            ++m_szCounter;
        }

        bool release()
        {
            --m_szCounter;
            if (0 == m_szCounter && nullptr != m_pPtrBase)
            {
                m_pPtrBase->destroy();
                deleteObject(m_pPtrBase, m_pPtrBase->getClassSize());
                m_pPtrBase = nullptr;
                return true;
            }
            return false;
        }

        size_t useCount() const
        {
            return m_szCounter;
        }

        _T* get() const
        {
            if (nullptr != m_pPtrBase)
            {
                return m_pPtrBase->get();
            }
            else
            {
                return nullptr;
            }
        }

    private:
        size_t          m_szCounter = 0;
        CPtrBase<_T>*   m_pPtrBase = nullptr;
    };


    //////////////////////////////////////////////////////////////////////////
    //@brief 持有类\0
    //@notice 这个类和std提供的shared_ptr最大区别是计数成员是非锁的, 而且不提供weak_ptr, 实现是为了去掉锁增加效率\0
    template<class _T>
    class shared_ptr
    {
    public:
        shared_ptr()
        {
            
        }

        template<class _U>
        shared_ptr(_U* _pPtr)
        {
            set(_pPtr);
            retain();
        }

        template<class _U, class _Deletor>
        shared_ptr(_U* _pPtr, _Deletor _deletor)
        {
            set<_U, _Deletor>(_pPtr, _deletor);
            retain();
        }

        ~shared_ptr()
        {
            release();
        }

        //移动构造函数\0
        shared_ptr(shared_ptr<_T>&& _rhs)
            : m_pCounter(_rhs.m_pCounter)
        {
            retain();
            _rhs.reset();
        }

        //拷贝构造\0
        shared_ptr(const shared_ptr<_T>& _rhs)
        {
            m_pCounter = _rhs.m_pCounter;
            retain();
        }

        //赋值构造\0
        shared_ptr& operator=(shared_ptr<_T>& _rhs)
        {
            release();
            m_pCounter = _rhs.m_pCounter;
            retain();
            return *this;
        }

        _T* get() const
        {
            if (nullptr != m_pCounter)
            {
                return m_pCounter->get();
            }
            else
            {
                return nullptr;
            }
        }

        _T* operator->() const
        {
            return get();
        }

        void reset()
        {
            release();
            m_pCounter = nullptr;
        }

        template<class _U>
        void reset(_U* _pPtr)
        {
            release();
            set<_U>(_pPtr);
            retain();
        }

        template<class _U, class _Deletor>
        void reset(_U* _pPtr, _Deletor _deletor)
        {
            release();
            set<_U, _Deletor>(_pPtr, _deletor);
            retain();
        }

        void swap(shared_ptr<_T>& _rhs)
        {
            CCounterBase<_T>* pTmp = _rhs.m_pCounter;
            _rhs.m_pCounter = m_pCounter;
            m_pCounter = pTmp;
        }

        bool unique() const
        {
            return (1 == use_count());
        }

        size_t use_count() const
        {
            if (nullptr != m_pCounter)
            {
                return m_pCounter->useCount();
            }
            else
            {
                return 0;
            }
        }

        bool operator==(const shared_ptr<_T>& _rhs) const
        {
            return (m_pCounter == _rhs.m_pCounter);
        }

        bool operator!=(const shared_ptr<_T>& _rhs) const
        {
            return !(*this == _rhs);
        }

    private:

        //////////////////////////////////////////////////////////////////////////
        //internal function, user use "reset" instead
        template<class _U>
        void set(_U* _pPtr)
        {
            if (nullptr != _pPtr)
            {
                m_pCounter = newObject<CCounterBase<_U>>(_pPtr);
            }
        }

        //////////////////////////////////////////////////////////////////////////
        //internal function, user use "reset" instead
        template<class _U, class _Deletor>
        void set(_U* _pPtr, _Deletor _deletor)
        {
            if (nullptr != _pPtr)
            {
                m_pCounter = newObject<CCounterBase<_U>>(_pPtr, _deletor);
            }
        }

        void retain()
        {
            if (nullptr != m_pCounter)
            {
                m_pCounter->retain();
            }
        }

        void release()
        {
            if (nullptr != m_pCounter)
            {
                if (m_pCounter->release())
                {
                    deleteObject(m_pCounter);
                    m_pCounter = nullptr;
                }
            }
        }

    private:
        CCounterBase<_T>*   m_pCounter = nullptr;
    };


}
