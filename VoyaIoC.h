
#pragma once

#include <unordered_map>
#include "VoyaUtil/VoyaNoCopyAble.h"
#include "VoyaUtil/VoyaAny.h"

namespace voya
{
    class CIoC : public CNoCopyAble
    {
        using ConstructMap = std::unordered_map<std::string, voya::CAny>;

    public:

        //注册类型\0
        template<class _T, typename ... Args>
        void registerType(const std::string& _strKey)
        {
            using TargetType = _T;
            auto iterFind = m_map.find(_strKey);
            if (iterFind == m_map.end())
            {
                std::function<TargetType*(Args...)> function = [](Args ... _args) {return new TargetType(_args ...); };
                m_map.emplace(_strKey, function);
            }
        }

        //注销类型\0
        void unregisterType(const std::string& _strKey)
        {
            auto iterFind = m_map.find(_strKey);
            if (iterFind != m_map.end())
            {
                m_map.erase(iterFind);
            }
        }

        //清空类型\0
        void clear()
        {
            m_map.clear();
        }

        //创建类型\0
        template<class _T, typename ... Args>
        _T* createType(const std::string& _strKey, Args ... _args)
        {
            using TargetType = _T;
            auto iterFind = m_map.find(_strKey);
            if (iterFind != m_map.end())
            {
                return iterFind->second.cast<std::function<TargetType*(Args...)>>()(_args...);
            }
            return nullptr;
        }

        //创建智能指针类型\0
        template<class _T, typename ... Args>
        std::shared_ptr<_T> createSharedPtrType(const std::string& _strKey, Args ... _args)
        {
            return std::shared_ptr<_T>(createType<_T, Args ...>(_strKey, _args ...));
        }

    private:
        ConstructMap    m_map;      //构造函数表\0

    };



}