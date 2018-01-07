
#pragma once

namespace voya
{
    /* 使用方法

    class 被访问者1;
    class 被访问者2;
    ...
    class 被访问者N;

    using 访问者基类名字 = voya::CVisitorBase<被访问者1, 被访问者2, ..., 被访问者N>::MyVisitors;
    using 被访问者基类名字 = voya::CAcceptorBase<访问者基类名字>;

    class 被访问者1 : public 被访问者基类名字
    {
        ACCEPT_IMPLEMENT(访问者基类名字);
    }

    class 被访问者2 : public 被访问者基类名字
    {
        ACCEPT_IMPLEMENT(访问者基类名字);
    }

    ...

    class 访问者 : public 访问者基类名字
    {
        virtual void visit(被访问者1& _instance)
        {
            //自定义代码
        }

        virtual void visit(被访问者2& _instance)
        {
            //自定义代码
        }

        ...
    }
    
    */



    template<typename ... _Types> class CVisitor;

    //变长模板递归定义访问者类型虚函数visit\0
    template<typename _T, typename ... _Types>
    class CVisitor<_T, _Types ...> : public CVisitor<_Types...>
    {
    public:
        //通过using避免隐藏基类的visit方法\0
        using CVisitor<_Types ...>::visit;

        virtual void visit(_T&) = 0;
    };
    template<typename _T>
    class CVisitor<_T>
    {
    public:
        virtual void visit(_T&) = 0;
    };

    //访问者基类模板\0
    template<typename ... _Types>
    class CVisitorBase
    {
    public:
        using MyVisitors = CVisitor<_Types...>;
    };

    //被访问者基类模板\0
    template<class _T>
    class CAcceptorBase
    {
    public:
        virtual void accept(_T&) = 0;
    };

    //被访问者accept虚函数实现\0
#define ACCEPT_IMPLEMENT(_VISITOR_CLASS_NAME) virtual void accept(_VISITOR_CLASS_NAME& _visitors){_visitors.visit(*this);}

}