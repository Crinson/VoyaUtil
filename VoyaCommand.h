
#pragma once

#include <functional>

namespace voya
{
    //命令通用类\0
    //用lambda函数封装所有参数类型的函数调用\0
    template<typename _ReturnType = void>
    class CCommand
    {
        using ReturnType = _ReturnType;
        using ReturnFunction = std::function<ReturnType()>;

    public:

        //非成员函数\0
        template<class _Function, 
                        class ... Args, 
                        class = typename std::enable_if<!std::is_member_function_pointer<_Function>::value>::type>
        void wrap(_Function&& _function, Args&& ... _args)
        {
            m_function = [&] {return _function(_args ...); };
        }

        //常量成员函数\0
        template<class _Return, class _Class, class ... DArgs, class _Ptr, class ... Args>
        void wrap( _Return(_Class::*f)(DArgs ...) const, _Ptr&& _ptr, Args&& ... _args)
        {
            m_function = [&, f] {return (*_ptr.*f)(std::forward<Args>(_args) ...); };
        }

        //非常量成员函数\0
        template<class _Return, class _Class, class ... DArgs, class _Ptr, class ... Args>
        void wrap( _Return(_Class::*f)(DArgs ...), _Ptr&& _ptr, Args&& ... _args)
        {
            m_function = [&, f] {return (*_ptr.*f)(std::forward<Args>(_args) ...); };
        }

        //易变非常量成员函数\0
        template<class _Return, class _Class, class ... DArgs, class _Ptr, class ... Args>
        void wrap(_Return(_Class::*f)(DArgs ...) volatile, _Ptr&& _ptr, Args&& ... _args)
        {
            m_function = [&, f] {return (*_ptr.*f)(std::forward<Args>(_args) ...); };
        }

        //易变常量成员函数\0
        template<class _Return, class _Class, class ... DArgs, class _Ptr, class ... Args>
        void wrap(_Return(_Class::*f)(DArgs ...) const volatile, _Ptr&& _ptr, Args&& ... _args)
        {
            m_function = [&, f] {return (*_ptr.*f)(std::forward<Args>(_args) ...); };
        }

        //运行命令\0
        ReturnType run()
        {
            return m_function();
        }

    private:
        ReturnFunction m_function;
    };



}