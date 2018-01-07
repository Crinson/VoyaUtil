
#pragma once

#include <typeindex>
#include <memory>

namespace voya
{
    //可以容纳任意数据的类型\0
    class CAny
    {
        /////////////////////////////////////////////////////////内部类 begin\0
        //数据基类\0
        class CBase;
        using uniPtrCBase = std::unique_ptr<CBase>;

        class CBase
        {
        public:
            virtual ~CBase() {}
            virtual uniPtrCBase clone() const = 0;
        };
        

        //数据保存类\0
        template<typename _T>
        class CSave : public CBase
        {
            using RealType = _T;
        public:
            template<typename _UType>
            CSave(_UType && _value) : m_value(std::forward<_UType>(_value))
            {
            }

            //克隆真实数据\0
            virtual uniPtrCBase clone() const
            {
                return uniPtrCBase(new CSave<_T>(m_value));
            }

            _T m_value;     //真实数据\0
        };
        /////////////////////////////////////////////////////////内部类 end\0

    public:
        //默认构造\0
        CAny() = default;

        //拷贝构造\0
        CAny(const CAny& _rhs)
            : m_pPtr(_rhs.clone())
            , m_typeIndex(_rhs.m_typeIndex)
        {

        }

        //右值同类构造\0
        CAny(CAny&& _rhs)
            : m_pPtr(std::move(_rhs.m_pPtr))
            , m_typeIndex(_rhs.m_typeIndex)
        {

        }

        //赋值函数\0
        CAny& operator=(const CAny& _rhs)
        {
            if (m_pPtr != _rhs.m_pPtr)
            {
                m_pPtr = _rhs.clone();
                m_typeIndex = _rhs.m_typeIndex;
            }
            return *this;
        }
        
        //克隆指智能指针\0
        uniPtrCBase clone() const
        {
            if (nullptr != m_pPtr)
            {
                return m_pPtr->clone();
            }
            return nullptr;
        }

        //对比类型是否相同\0
        template<class _UType>
        bool isSameType() const
        {
            auto otherTypeIndex = std::type_index(typeid(typename std::decay<_UType>::type));
            return (m_typeIndex ==  otherTypeIndex);
        }

        //是否存有数据\0
        bool isEmpty() const
        {
            return (nullptr == m_pPtr);
        }

        //右值构造\0
        template< typename _ValueType, class = typename std::enable_if< !std::is_same<typename std::decay<_ValueType>::type, CAny>::value, _ValueType>::type >
        CAny(_ValueType&& _value)
            : m_pPtr(new CSave<typename std::decay<_ValueType>::type>(std::forward<_ValueType>(_value)))
            , m_typeIndex(typeid(typename std::decay<_ValueType>::type))
        {

        }

        //类型转换\0
        template< class _UType >
        _UType& cast()
        {
            if (!isSameType<_UType>())
            {
                throw std::bad_cast();
            }
            auto data = dynamic_cast<CSave<_UType>*>(m_pPtr.get());
            return data->m_value;
        }


    private:
        uniPtrCBase          m_pPtr = nullptr;          //数据内容智能指针\0
        std::type_index     m_typeIndex = std::type_index(typeid(void));        //数据类型信息\0
    };

}