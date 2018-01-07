
#pragma once

namespace voya
{

    class CCounter
    {
    public:
        inline void retain()
        {
            ++m_iCouter;
        }

        void release()
        {
            --m_iCouter;
            if (m_iCouter <= 0)
            {
                delete this;
            }
        }

        inline int getReferenceCount() const
        {
            return m_iCouter;
        }

        virtual ~CCounter(){}

    private:
        int    m_iCouter = 0;
    };


    //_CCounter 需要传入CCounter的派生类型\0
    //CHolder 不能作为基类，因为析构函数为非虚函数，会导致不明问题\0
    template<class _CCounter>
    class CHolder
    {
    public:
        using HolderType = _CCounter;

        CHolder()
        {

        }

        CHolder(HolderType* _pPtr)
        {
            retain(_pPtr);
            m_pPtr = _pPtr;
        }

        //移动构造函数\0
        CHolder(CHolder&& _rhs)
            : m_pPtr(_rhs.m_pPtr)
        {
            _rhs.m_pPtr = nullptr;
        }

        //拷贝构造\0
        CHolder(const CHolder& _rhs)
        {
            retain(_rhs.m_pPtr);
            m_pPtr = _rhs.m_pPtr;
        }

        //赋值构造\0
        CHolder& operator=(const CHolder& _rhs)
        {
            retain(_rhs.m_pPtr);
            release(m_pPtr);
            m_pPtr = _rhs.m_pPtr;
            return *this;
        }


        CHolder& operator=(HolderType* _pPtr)
        {
            retain(_pPtr);
            release(m_pPtr);
            m_pPtr = _pPtr;
            return *this;
        }

        HolderType* operator->() const
        {
            return m_pPtr;
        }

        bool isNull() const
        {
            return nullptr == m_pPtr;
        }

        void reset()
        {
            release(m_pPtr);
            m_pPtr = nullptr;
        }

        HolderType* get() const
        {
            return m_pPtr;
        }

        int getReferenceCount() const
        {
            return nullptr == m_pPtr ? 0 : m_pPtr->getReferenceCount();
        }

        bool operator==(const CHolder& _rhs) const
        {
            return m_pPtr == _rhs.m_pPtr;
        }

        ~CHolder()
        {
            release(m_pPtr);
            m_pPtr = nullptr;
        }

    private:
        inline void retain(HolderType* _pPtr)
        {
            if (nullptr != _pPtr)
            {
                _pPtr->retain();
            }
        }

        inline void release(HolderType* _pPtr)
        {
            if (nullptr != _pPtr)
            {
                _pPtr->release();
            }
        }

    private:
        HolderType* m_pPtr = nullptr;
    };


}
