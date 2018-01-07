
#pragma once

#include <string>
#include <unordered_map>
#include <list>
#include <list>
#include <functional>
#include "VoyaUtil/VoyaAssert.h"
#include "VoyaUtil/VoyaAny.h"
#include "VoyaUtil/VoyaToFunction.h"

namespace voya
{
    /*用法
        enum class eTest
        {
            eTest_Start,
            eTest_Middle,
            eTest_End,
        };

        voya::CStateMachine<eTest> state(eTest::eTest_Start);
        state.addEvent("aaaa",
                        [](eTest _from, eTest _to, void* _i)
                        {
                            CCLOG("haha");
                        }
                        ,eTest::eTest_End,
                        eTest::eTest_Start,
                        eTest::eTest_Middle);
        state.setStateLeaveCallBack(eTest::eTest_Start,
                        [](eTest _from, eTest _to)
                        {
                            CCLOG("leave");
                            return true;
                        });
        state.setStateEnterCallBack(eTest::eTest_End,
                        [](eTest _from, eTest _to)
                        {
                            CCLOG("enter");
                            return true;
                        });
        state.doEvent("aaaa", (void*)8);
        bool b = state.isCanDoEvent("aaaa");
        state.removeEvent("aaaa");
        state.clearEvent();
    */

    //事件块\0
    template<class StateEnum>
    class _CEventBlock
    {
    public:
        using FromList = std::list<StateEnum>;
        using CallBackFunction = CAny;

        template< typename ... Args >
        _CEventBlock(const std::string& _strEventName, StateEnum _to, Args&& ... _froms)
            : m_strEventName(_strEventName)
        {
            m_to = _to;
            pushBackFrom(std::forward<Args>(_froms)...);
        }

        //获得事件名字\0
        const std::string& getEventName() const { return m_strEventName; }

        //获得目的状态\0
        StateEnum getToState() const { return m_to; }

        //获得起始状态列表\0
        const FromList& getFromStates(){ return m_froms; }

        //设置回调函数\0
        void setCallBack(const CallBackFunction& _callBack)
        {
            m_callBack = _callBack;
        }

        //调用回调函数\0
        template< typename ... Args >
        bool callCallBack(StateEnum _from, StateEnum _to, Args&& ... _args)
        {
            if (!m_callBack.isEmpty())
            {
                using FunctionType = std::function<void(StateEnum, StateEnum, Args...)>;
                FunctionType cb = m_callBack.cast<FunctionType>();
                cb(_from, _to, std::forward<Args>(_args)...);
                return true;
            }
            return false;
        }

    private:
        //增加起始状态\0
        template<typename ... Args>
        void pushBackFrom(StateEnum _from, Args&& ... _args)
        {
            m_froms.push_back(_from);
            pushBackFrom(std::forward<Args>(_args)...);
        }

        void pushBackFrom() {}

    private:
        std::string         m_strEventName;         //事件名字\0
        FromList            m_froms;                //起始状态列表\0
        StateEnum           m_to;                   //目的状态\0
        CallBackFunction    m_callBack;             //回调函数\0
    };

    //状态块\0
    template<class StateEnum>
    class _CStateBlock
    {
        using CallBackFunction = std::function<bool(StateEnum, StateEnum)>;

    public:
        _CStateBlock(StateEnum _eState)
        {
            m_eState = _eState;
        }

        void setEnterCallBack(const CallBackFunction& _callBack){ m_enterCallBack = _callBack; }
        bool callEnterCallBack(StateEnum _from, StateEnum _to)
        {
            if (nullptr != m_enterCallBack)
            {
                return m_enterCallBack(_from, _to);
            }
            return true;
        }

        void setLeaveCallBack(const CallBackFunction& _callBack){ m_leaveCallBack = _callBack; }
        bool callLeaveCallBack(StateEnum _from, StateEnum _to)
        {
            if (nullptr != m_leaveCallBack)
            {
                return m_leaveCallBack(_from, _to);
            }
            return true;
        }

    private:
        StateEnum           m_eState;
        CallBackFunction    m_enterCallBack;
        CallBackFunction    m_leaveCallBack;

    };


    //状态机\0
    template<class StateEnum>
    class CStateMachine
    {
    public:
        using CEventBlock = _CEventBlock<StateEnum>;
        using CStateBlock = _CStateBlock<StateEnum>;
        using EventMapType = std::unordered_map<std::string, CEventBlock*>;
        using StateMapType = std::unordered_map<int, CStateBlock*>;
        using StateCallBackFunction = std::function<bool(StateEnum, StateEnum)>;

    public:
        CStateMachine(StateEnum _eInitState)
            : m_stateNow(_eInitState)
        {

        }

        ~CStateMachine()
        {
            clearEvent();
            clearState();
        }

        //是否可以执行事件\0
        bool isCanDoEvent(const std::string& _strEventName) const
        {
            if (m_bInTranslation)
            {
                voya_log_warning("StateMachine is in Translation, Event:%s dosn't call!!!", _strEventName.c_str());
                return false;
            }
            CEventBlock* pEvent = getEventBlock(_strEventName);
            if (nullptr == pEvent)
            {
                return false;
            }

            if (m_stateNow == pEvent->getToState())
            {
                return false;
            }

            const typename CEventBlock::FromList& froms = pEvent->getFromStates();
            for (auto iter = froms.begin(); iter != froms.end(); ++iter)
            {
                if (*iter == m_stateNow)
                {
                    return true;
                }
            }
            return false;
        }

        //增加事件\0
        template< typename FunctionType, typename ... Args >
        bool addEvent(const std::string& _strEventName, FunctionType&& _callBack, StateEnum _to, Args&& ... _froms)
        {
            CEventBlock* pEvent = getEventBlock(_strEventName);
            if (nullptr != pEvent)
            {
                return true;
            }
            pEvent = new CEventBlock(_strEventName, _to, std::forward<Args>(_froms)...);
            auto strFunction = toStlFunction(std::forward<FunctionType>(_callBack));
            pEvent->setCallBack(CAny(strFunction));
            m_events[_strEventName] = pEvent;
            return true;
        }

        //删除事件\0
        void removeEvent(const std::string& _strEventName)
        {
            CEventBlock* pEvent = getEventBlock(_strEventName);
            if (nullptr == pEvent)
            {
                return;
            }

            delete pEvent;
            m_events.erase(_strEventName);
        }

        template<typename ... Args>
        bool doEvent(const std::string& _strEvent, Args&& ... _args)
        {
            if (!isCanDoEvent(_strEvent))
            {
                return false;
            }
            CEventBlock* pEvent = getEventBlock(_strEvent);
            if (nullptr == pEvent)
            {
                return false;
            }

            m_bInTranslation = true;
            //退出原来状态\0
            CStateBlock* pStateBlock = getStateBlock(m_stateNow);
            if (nullptr != pStateBlock && !pStateBlock->callLeaveCallBack(m_stateNow, pEvent->getToState()))
            {
                return false;
            }

            //进入新的状态\0
            pStateBlock = getStateBlock(pEvent->getToState());
            if (nullptr != pStateBlock && !pStateBlock->callEnterCallBack(m_stateNow, pEvent->getToState()))
            {
                return false;
            }

            auto lastState = pEvent->getToState();
            m_stateNow = lastState;
            m_bInTranslation = false;
            
            //调用事件\0
            pEvent->callCallBack(lastState, pEvent->getToState(), std::forward<Args>(_args)...);
            
            
            return true;
        }

        //清理事件\0
        void clearEvent()
        {
            auto iter = m_events.begin();
            while (iter != m_events.end())
            {
                delete iter->second;
                ++iter;
            }
            m_events.clear();
        }

        //清理状态事件\0
        void clearState()
        {
            auto iter = m_states.begin();
            while (iter != m_states.end())
            {
                delete iter->second;
                ++iter;
            }
            m_states.clear();
        }

        //设置进入状态回调函数\0
        void setStateEnterCallBack(StateEnum _eState, const StateCallBackFunction& _callBack)
        {
            CStateBlock* pBlock = getStateBlock(_eState);
            if (nullptr == pBlock)
            {
                pBlock = addStateBlock(_eState);
            }
            pBlock->setEnterCallBack(_callBack);
        }

        //设置退出状态回调函数\0
        void setStateLeaveCallBack(StateEnum _eState, const StateCallBackFunction& _callBack)
        {
            CStateBlock* pBlock = getStateBlock(_eState);
            if (nullptr == pBlock)
            {
                pBlock = addStateBlock(_eState);
            }
            pBlock->setLeaveCallBack(_callBack);
        }

        //获得状态\0
        StateEnum getState() const
        {
            return m_stateNow;
        }

        //是否在状态中\0
        bool isState(StateEnum _eState) const
        {
            return getState() == _eState;
        }

    private:
        //获得事件\0
        CEventBlock* getEventBlock(const std::string& _strEventName) const
        {
            if (_strEventName.empty())
            {
                return nullptr;
            }
            auto iterFind = m_events.find(_strEventName);
            if (iterFind == m_events.end())
            {
                return nullptr;
            }
            return iterFind->second;
        }

        //获得状态块\0
        CStateBlock* getStateBlock(StateEnum _eState) const
        {
            int iState = (int)_eState;
            auto iterFind = m_states.find(iState);
            if (iterFind == m_states.end())
            {
                return nullptr;
            }
            return iterFind->second;
        }

        //增加状态块\0
        CStateBlock* addStateBlock(StateEnum _eState)
        {
            int iState = (int)_eState;
            auto iterFind = m_states.find(iState);
            if (iterFind == m_states.end())
            {
                CStateBlock* pBlock = new CStateBlock(_eState);
                m_states[iState] = pBlock;
                return pBlock;
            }
            return iterFind->second;
        }


    private:
        EventMapType    m_events;                       //事件表\0
        StateMapType    m_states;                       //状态表\0
        StateEnum       m_stateNow;                     //现在的状态\0
        bool            m_bInTranslation = false;       //状态是否在变换中\0
    };


}

