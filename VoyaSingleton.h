
#pragma once

#include <type_traits>
#include <utility>
#include "VoyaUtil/VoyaNoCopyAble.h"

namespace voya
{
    //单例(默认没有构造参数)
	template<class _T, bool _NoConstructParam = true> class CSingleton;

	//构造函数没有参数单例\0
    template<class _T>
    class CSingleton<_T, true> : public CNoCopyAble
    {
	public:
        using SingletonType = _T;

        //获取单例\0
        static SingletonType* instance()
        {
			if (nullptr == s_pInstance)
			{
				s_pInstance = new SingletonType();
			}
            return s_pInstance;
        }

        //删除单例\0
        static void destroy()
        {
            if (nullptr != s_pInstance)
            {
                delete s_pInstance;
                s_pInstance = nullptr;
            }
        }

    private:
        static SingletonType*  s_pInstance;

    };

    template<class _T> _T* CSingleton<_T, true>::s_pInstance = nullptr;



	//构造函数有参数单例，此类单例使用前需要先调用init函数初始化\0
	template<class _T>
	class CSingleton<_T, false> : public CNoCopyAble
	{
	public:
		using SingletonType = _T;

		//获取单例\0
		static SingletonType* instance()
		{
			return s_pInstance;
		}

		//变长模板参数完美转发构造\0
		template<typename ... Args>
		static SingletonType* init(Args&& ... _args)
		{
			if (nullptr == s_pInstance)
			{
				s_pInstance = new SingletonType(std::forward<Args>(_args) ...);
			}
			return s_pInstance;
		}

		//删除单例\0
		static void destroy()
		{
			if (nullptr != s_pInstance)
			{
				delete s_pInstance;
				s_pInstance = nullptr;
			}
		}

	private:
		static SingletonType*  s_pInstance;

	};

	template<class _T> _T* CSingleton<_T, false>::s_pInstance = nullptr;

}
