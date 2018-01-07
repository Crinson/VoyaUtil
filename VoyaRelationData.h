
#pragma once

#include <functional>
#include "VoyaUtil/VoyaSTL.h"

namespace voya
{
    //请勿以此文件下的类作为基类，类内的析构函数为非虚函数，派生可能会产生问题\0

    template<typename _IndexType, typename _DataType>
    class CLevelMap
    {
    public:
        using INDEX_TYPE = _IndexType;
        using DATA_TYPE = _DataType;
        using MAP_TYPE = voya::unordered_map<INDEX_TYPE, DATA_TYPE*>;

        DATA_TYPE* add(const INDEX_TYPE& _index)
        {
            DATA_TYPE* pData = get(_index);
            if (nullptr == pData)
            {
                pData = newObject<DATA_TYPE>();
                m_datas.emplace(_index, pData);
            }
            return pData;
        }

        DATA_TYPE* get(const INDEX_TYPE& _index, bool _bCreate = false)
        {
            auto&& iter = m_datas.find(_index);
            if (iter != m_datas.end())
            {
                return iter->second;
            }
            else if (_bCreate)
            {
                return add(_index);
            }
            else
            {
                return nullptr;
            }
        }

        void remove(const INDEX_TYPE& _index)
        {
            DATA_TYPE* pData = get(_index);
            if (nullptr != pData)
            {
                deleteObject(pData);
                pData = nullptr;
                m_datas.erase(m_datas.find(_index));
            }
        }

        void clear()
        {
            for (auto&& iter : m_datas)
            {
                deleteObject(iter.second);
            }
            m_datas.clear();
        }

        int size() const
        {
            return m_datas.size();
        }

        bool empty() const
        {
            return m_datas.empty();
        }

        DATA_TYPE& operator[](const INDEX_TYPE& _index)
        {
            return *add(_index);
        }

        const MAP_TYPE& getChilds() const
        {
            return m_datas;
        }

        ~CLevelMap()
        {
            clear();
        }

    private:
        MAP_TYPE m_datas;

    };



    //////////////////////////////////////////////////////////////////////////
    template<typename _IndexType, typename _DataType>
    class CTreeMap
    {
    public:
        using INDEX_TYPE = _IndexType;
        using DATA_TYPE = _DataType;

        class _CNew
        {
            using LEVEL_MAP = CLevelMap<INDEX_TYPE, _CNew>;

        public:
            _CNew* add(const INDEX_TYPE& _index)
            {
                if (nullptr == m_pChilds)
                {
                    m_pChilds = newObject<LEVEL_MAP>();
                }
                return m_pChilds->add(_index);
            }

            _CNew* get(const INDEX_TYPE& _index, bool _bCreate = false)
            {
                if (nullptr == m_pChilds)
                {
                    if (_bCreate)
                    {
                        return add(_index);
                    }
                    else
                    {
                        return nullptr;
                    }
                }
                else
                {
                    return m_pChilds->get(_index, _bCreate);
                }
            }

            const LEVEL_MAP& getChilds()
            {
                if (nullptr == m_pChilds)
                {
                    m_pChilds = newObject<LEVEL_MAP>();
                }
                return *m_pChilds;
            }

            void remove(const INDEX_TYPE& _index)
            {
                if (nullptr != m_pChilds)
                {
                    m_pChilds->remove(_index);
                }
            }

            void clear()
            {
                if (nullptr != m_pChilds)
                {
                    deleteObject(m_pChilds);
                    m_pChilds = nullptr;
                }
            }

            int size() const
            {
                if (nullptr != m_pChilds)
                {
                    return m_pChilds->size();
                }
                else
                {
                    return 0;
                }
            }

            int fullSize() const
            {
                if (nullptr == m_pChilds)
                {
                    return 0;
                }
                else
                {
                    int iSize = m_pChilds->size();
                    auto& childs = m_pChilds->getChilds();
                    for (auto&& iter : childs)
                    {
                        iSize += iter.second->fullSize();
                    }
                    return iSize;
                }
            }

            bool empty() const
            {
                if (nullptr != m_pChilds)
                {
                    return m_pChilds->empty();
                }
                else
                {
                    return true;
                }
            }

            _CNew& operator[](const INDEX_TYPE& _index)
            {
                return *add(_index);
            }

            void emit(const INDEX_TYPE& _index, std::function<void(const INDEX_TYPE&, _CNew*)>&& _call)
            {
                _call(_index, this);
                if (nullptr != m_pChilds)
                {
                    for (auto&& iter : m_pChilds->getChilds())
                    {
                        iter.second->emit(iter.first, std::move(_call));
                    }
                }
            }

            ~_CNew()
            {
                clear();
                clearData();
            }

            DATA_TYPE* getData(bool _bCreate = false)
            {
                if (nullptr != m_pData)
                {
                    return m_pData;
                }
                else
                {
                    if (_bCreate)
                    {
                        m_pData = newObject<DATA_TYPE>();
                        return m_pData;
                    }
                    else
                    {
                        return nullptr;
                    }
                }
            }

            void setData(DATA_TYPE _pData)
            {
                *getData(true) = _pData;
            }

            void clearData()
            {
                if (nullptr != m_pData)
                {
                    deleteObject(m_pData);
                    m_pData = nullptr;
                }
            }

        private:
            DATA_TYPE* m_pData = nullptr;
            LEVEL_MAP* m_pChilds = nullptr;
        };

        using NEW_CLASS_TYPE = _CNew;
        using EMIT_TYPE = std::function<void(const INDEX_TYPE&, _CNew*)>;

        NEW_CLASS_TYPE* getRoot()
        {
            if (nullptr == m_pDatas)
            {
                m_pDatas = newObject<NEW_CLASS_TYPE>();
            }
            return m_pDatas;
        }

        void clearRoot()
        {
            if (nullptr != m_pDatas)
            {
                deleteObject(m_pDatas);
                m_pDatas = nullptr;
            }
        }

        ~CTreeMap()
        {
            clearRoot();
        }

    private:
        NEW_CLASS_TYPE*  m_pDatas = nullptr;
    };

// 
// 
//     //////////////////////////////////////////////////////////////////////////
//     void testCLevelMap()
//     {
//         CLevelMap<int, CLevelMap<int, int>> data;
//         int* pError = nullptr;
// 
//         //case 1
//         data[3][5] = 7;
//         if (data[3][5] != *data.get(3)->get(5))
//         {
//             *pError = 1;
//         }
// 
//         //case 7
//         int iSize = data.size();
//         auto temp = data.getChilds();
//         for (auto&& iter : temp)
//         {
//             iSize += iter.second->size();
//         }
//         if (iSize != 2)
//         {
//             *pError = 7;
//         }
// 
//         //case 3
//         data.get(3)->remove(5);
//         if (data.get(3)->size() != 0)
//         {
//             *pError = 3;
//         }
// 
//         //case 4
//         data.clear();
//         if (data.size() != 0)
//         {
//             *pError = 4;
//         }
// 
//         //case 5
//         if (!data.empty())
//         {
//             *pError = 5;
//         }
//     }
// 
//     void testCTreeMap()
//     {
//         CTreeMap<std::string, bool> wordMap;
//         int* pError = nullptr;
// 
//         //前置依赖测试\0
//         testCLevelMap();
// 
//         //case 1
//         auto* pBool = wordMap.getRoot()->getData();
//         if (nullptr != pBool)
//         {
//             *pError = 1;
//         }
// 
//         //case 2
//         pBool = wordMap.getRoot()->getData(true);
//         if (nullptr == pBool)
//         {
//             *pError = 2;
//         }
// 
//         //case 3
//         *pBool = false;
//         if (false != *wordMap.getRoot()->getData())
//         {
//             *pError = 3;
//         }
// 
//         //case 4
//         auto* pChild = wordMap.getRoot()->get("a", true);
//         if (nullptr == pChild)
//         {
//             *pError = 4;
//         }
// 
//         //case 5
//         auto* pChild3 = wordMap.getRoot()->get("a", true)->get("b", true)->get("c", true);
//         if (nullptr == pChild3)
//         {
//             *pError = 5;
//         }
// 
//         //case 6
//         auto pChild4Data = (*wordMap.getRoot())["a"]["b"]["c"].getData(true);
//         *pChild4Data = true;
//         if (!(*pChild3->getData()))
//         {
//             *pError = 6;
//         }
// 
//         //case 7
//         (*wordMap.getRoot())["a"]["d"];
//         (*wordMap.getRoot())["a"]["b"]["e"];
//         int iSize = wordMap.getRoot()->fullSize();
//         if (iSize != 5)
//         {
//             *pError = 7;
//         }
// 
//         //case 8
//         CTreeMap<std::string, bool>::EMIT_TYPE cb = [](CTreeMap<std::string, bool>::NEW_CLASS_TYPE* _pObject)
//         {
//             bool* pBool = _pObject->getData(true);
//             *pBool = false;
//         };
//         wordMap.getRoot()->emit(std::move(cb));
// 
//         if (*(*wordMap.getRoot())["a"]["b"]["c"].getData())
//         {
//             *pError = 8;
//         }
// 
//     }

}

