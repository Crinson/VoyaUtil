
#pragma once

#include <functional>
#include "VoyaUtil/VoyaNoCopyAble.h"

namespace voya
{
#define SCOPE_GUARD_LINENAME_CAT(_name, _line) _name##_line
#define SCOPE_GUARD_LINENAME(_name, _line) SCOPE_GUARD_LINENAME_CAT(_name, _line)

#define SCOPE_GUARD_EXIT(_callback) CScopeGuard SCOPE_GUARD_LINENAME(EXIT, __LINE__)(std::move(_callback))

#define SCOPE_GUARD_DELETE_PTR(_PTR) SCOPE_GUARD_EXIT(([&_PTR]{ if(nullptr != _PTR) {delete _PTR; _PTR = nullptr;} }))
#define SCOPE_GUARD_DELETE_ARRAY(_PTR) SCOPE_GUARD_EXIT(([&_PTR]{ if(nullptr != _PTR) {delete[] _PTR; _PTR = nullptr;} }))

	//对CScopeGuard的简易使用可用lambda表达式，例如
	// 	FILE* pFile = fopen("abc.txt", "r");
	// 	SCOPE_GUARD_EXIT([&]
	// 	{
	// 		if (nullptr != pFile)
	// 		{
	// 			fclose(pFile);
	// 			pFile = nullptr;
	// 		}
	// 	});
	class CScopeGuard : public CNoCopyAble
	{
	public:
		typedef std::function< void() > CallBack;

		explicit CScopeGuard(CallBack&& _callback)
			: m_callback(std::forward<CallBack>(_callback))
			, m_bDismissed(false)
		{

		}

		~CScopeGuard()
		{
			if (!m_bDismissed)
			{
				m_callback();
			}
		}

		//解除函数回调
		void dismiss()
		{
			m_bDismissed = true;
		}

	private:
		bool			m_bDismissed;		//是否解除函数回调
		CallBack	m_callback;				//函数回调
	};


}