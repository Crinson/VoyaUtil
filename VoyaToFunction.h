
#pragma once

#include <functional>
#include <tuple>

namespace voya
{

    template<typename _T>
    class CFunctionTraits;

    //普通函数\0
    template<typename _ReturnType, typename ... Args>
    class CFunctionTraits<_ReturnType(Args...)>
    {
        typedef _ReturnType  functionType(Args...);
        
        enum { sizeOfArgs = sizeof...(Args) };
        template<size_t _ArgsSize>
        class CArgs
        {
            static_assert(_ArgsSize < sizeOfArgs, "index is out of range");
        public:
            using type = typename std::tuple_element<_ArgsSize, std::tuple<Args...>>::type;
        };

    public:
		typedef std::function<functionType> stlFunctionType;
		typedef _ReturnType(*functionPointer)(Args...);

    };

	//函数指针\0
	template<typename _ReturnType, typename ... Args>
	class CFunctionTraits<_ReturnType(*)(Args ...)> 
		: public CFunctionTraits<_ReturnType(Args...)>
	{

	};

	//std::function\0
	template<typename _ReturnType, typename ... Args>
	class CFunctionTraits<std::function<_ReturnType(Args...)>> 
		: public CFunctionTraits<_ReturnType(Args...)>
	{

	};

	//普通成员函数\0
	template<typename _ReturnType, typename _ClassType, typename ... Args>
	class CFunctionTraits<_ReturnType(_ClassType::*)(Args...)> 
		: public CFunctionTraits<_ReturnType(Args...)>
	{

	};

	//const成员函数\0
	template<typename _ReturnType, typename _ClassType, typename ... Args>
	class CFunctionTraits<_ReturnType(_ClassType::*)(Args...) const> 
		: public CFunctionTraits<_ReturnType(Args...)>
	{

	};

	//volatile成员函数\0
	template<typename _ReturnType, typename _ClassType, typename ... Args>
	class CFunctionTraits<_ReturnType(_ClassType::*)(Args...) volatile> 
		: public CFunctionTraits<_ReturnType(Args...)>
	{

	};

	//const volatile成员函数\0
	template<typename _ReturnType, typename _ClassType, typename ... Args>
	class CFunctionTraits<_ReturnType(_ClassType::*)(Args...) const volatile> 
		: public CFunctionTraits<_ReturnType(Args...)>
	{

	};

	//函数对象\0
	template<typename _CallAble>
	class CFunctionTraits : public CFunctionTraits<decltype(&_CallAble::operator())>
	{

	};


	//函数转换为stl::function\0
	template<typename _Function>
	typename CFunctionTraits<_Function>::stlFunctionType toStlFunction(const _Function& _lambda)
	{
		return static_cast<typename CFunctionTraits<_Function>::stlFunctionType>(_lambda);
	}
	template<typename _Function>
	typename CFunctionTraits<_Function>::stlFunctionType toStlFunction(_Function&& _lambda)
	{
		return static_cast<typename CFunctionTraits<_Function>::stlFunctionType>(std::forward<_Function>(_lambda));
	}

	//函数转换为函数指针\0
	template<typename _Function>
	typename CFunctionTraits<_Function>::functionPointer toFunctionPointer(const _Function& _lambda)
	{
		return static_cast<typename CFunctionTraits<_Function>::functionPointer>(_lambda);
	}
	template<typename _Function>
	typename CFunctionTraits<_Function>::functionPointer toFunctionPointer(_Function&& _lambda)
	{
		return static_cast<typename CFunctionTraits<_Function>::functionPointer>(std::forward<_Function>(_lambda));
	}



}
