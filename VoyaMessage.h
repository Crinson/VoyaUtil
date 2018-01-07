
#pragma once

#include <map>
#include "VoyaUtil/VoyaAny.h"
#include "VoyaUtil/VoyaToFunction.h"

namespace voya
{

    class CMessage
    {
        using MessageMap = std::multimap<std::string, voya::CAny>;

    public:
        template<typename _Function>
		void registerMessage(const std::string& _strMessage, _Function&& _function)
        {
			auto stlFunction = toStlFunction(std::forward<_Function>(_function));
			auto strSign = _strMessage + typeid(stlFunction).name();
			m_map.emplace(strSign, CAny(stlFunction));
        }

		template<typename _Function>
		void registerMessage(const std::string& _strMessage,const _Function& _function)
		{
			auto stlFunction = toStlFunction(_function);
			auto strSign = _strMessage + typeid(stlFunction).name();
			m_map.emplace(strSign, CAny(stlFunction));
		}

		template<typename _Function>
		void unregisterMessage(const std::string& _strMessage, _Function&& _function)
		{
			auto strSign = _strMessage + typeid(toStlFunction(std::forward<_Function>(_function))).name();
			auto range = m_map.equal_range(strSign);
			m_map.erase(range.first, range.second);
		}

		template<typename _Function>
		void unregisterMessage(const std::string& _strMessage,const _Function& _function)
		{
			auto strSign = _strMessage + typeid(toStlFunction(_function)).name();
			auto range = m_map.equal_range(strSign);
			m_map.erase(range.first, range.second);
		}

		template<typename ... Args>
		void sendMessage(const std::string& _strMessage, Args&& ... _args)
		{
			auto strSign = _strMessage + typeid(std::function<void(Args...)>).name();
			auto range = m_map.equal_range(strSign);
			for (auto iter = range.first; iter != range.second; ++iter)
			{
				auto function = iter->second.cast<std::function<void(Args...)>>();
                function(std::forward<Args>(_args)...);
			}
		}

    private:
        MessageMap m_map;

    };



}