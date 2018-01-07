
#pragma once

#include <unordered_map>
#include <type_traits>
#include "VoyaUtil/VoyaNoCopyAble.h"

namespace voya
{
    using EVENT_ID = int;
    
    //事件通知类\0
    template<typename _T>
    class CEvent : public CNoCopyAble
    {
        using EVENT_FUNCTION = _T;
        using EVENT_MAP = std::unordered_map<EVENT_ID, EVENT_FUNCTION>;

    public:
        //插入事件响应\0
        //成员函数使用std::bind
        EVENT_ID insert(const EVENT_FUNCTION& _function)
        {
            return _insert(_function);
        }

        //插入事件响应\0
        EVENT_ID insert(EVENT_FUNCTION&& _function)
        {
            return _insert(_function);
        }

        //删除事件响应\0
        void erase(EVENT_ID _id)
        {
            m_map.erase(_id);
        }

        //清空事件响应\0
        void clear()
        {
            m_map.clear();
            m_uniqueID = 1;
        }

        //获得事件响应数量\0
        int size()
        {
            return m_map.size();
        }

        //发送事件通知\0
        template<typename ... Args>
        void notify(Args&& ... _args)
        {
            for (auto& it : m_map)
            {
                it.second(std::forward<Args>(_args)...);
            }
        }

    private:
        template<typename _FUNCTION>
        EVENT_ID _insert(_FUNCTION&& _function)
        {
            EVENT_ID retID = m_uniqueID;
            m_map.emplace(m_uniqueID++, std::forward<_FUNCTION>(_function));
            return retID;
        }

    private:
        EVENT_MAP   m_map;                //事件表\0  
        EVENT_ID       m_uniqueID = 1;  //事件ID，不断递增\0
    };

}