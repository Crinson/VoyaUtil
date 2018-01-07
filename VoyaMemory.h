
#pragma once

#include <vector>
#include "VoyaUtil/VoyaAllocator.h"

namespace voya
{
    //内存池\0
    class CMemoryPool
    {
    public:
        static const size_t AUTO_CLASS_SIZE = 0;

        //使用默认的内存策略\0
        CMemoryPool()
        {
            //Block大小递进表\0
            static const std::vector<size_t> MEMORY_POOL_BLOCK_SIZE =
            {
                //以4增长\0
                4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48,
                52, 56, 60, 64, 68, 72, 76, 80, 84, 88, 92, 96,
                //以32增长\0
                128, 160, 192, 224, 256, 288, 320, 352, 384,
                416, 448, 480, 512, 544, 576, 608, 640,
                //以64增长\0
                704, 768, 832, 896, 960, 1024,
                //以128增长\0
                1152, 1280, 1408, 1536, 1664, 1792, 1920, 2048
            };
            reset(MEMORY_POOL_PAGE_SIZE, MEMORY_POOL_PAGE_ALGIN, MEMORY_POOL_BLOCK_SIZE);
        }

        //@brief 使用自定义内存分配策略
        //@param _szPageSize 分页大小
        //@param _szAlgin 对齐大小
        //@param _vecBlockSize 分配器策略(切记单个Block不要太大, 否则映射表会很大)\0
        CMemoryPool(size_t _szPageSize, size_t _szAlgin, const std::vector<size_t>& _vecBlockSize)
        {
            reset(_szPageSize, _szAlgin, _vecBlockSize);
        }

        ~CMemoryPool()
        {
            if (0 != usedBytes())
            {
                voya_log_warning("%s memory leak, there're %d byte datas havn't return to system allocator", m_strPoolTag.c_str(), usedBytes());
            }
            else
            {
                voya_log("%s no memory leak", m_strPoolTag.c_str());
            }
        }

        void setMemoryPoolTag(const std::string& _strTag)
        {
            m_strPoolTag = _strTag;
        }

        template<class _T, class ... _Args>
        _T* borrowObject(_Args ... _args)
        {
            return new(allocate(sizeof(_T))) _T(_args ...);
        }

        //提供_ObjectSize可选参数的原因是, 不能根据基类类型推导出派生类型的大小, 
        //所以在保存着基类指针的类型释放的时候, 需要提供正确的内存大小以正确释放\0
        template<class _T>
        void returnObject(_T* _pObject, size_t _ObjectSize)
        {
            if (nullptr == _pObject)
            {
                return;
            }
            reinterpret_cast<_T*>(_pObject)->~_T();
            if (AUTO_CLASS_SIZE == _ObjectSize)
            {
                deallocate(_pObject, sizeof(_T));
            }
            else
            {
                deallocate(_pObject, _ObjectSize);
            }
        }

#ifdef MEMORY_POOL_DEBUG
        void* allocate(size_t _szData, const char* _pszClassName = nullptr)
#else
        void* allocate(size_t _szData)
#endif
        {
            if (_szData >= m_Size2Allocator.size())
            {
                m_szSystemAllocate += _szData;
                return ::malloc(_szData);
            }
            else
            {
#ifdef MEMORY_POOL_DEBUG
                return m_Allocators[m_Size2Allocator[_szData]].allocate(_pszClassName);
#else
                return m_Allocators[m_Size2Allocator[_szData]].allocate();
#endif
                
            }
        }

        void deallocate(void* _pPtr, size_t _szData)
        {
            if (nullptr == _pPtr)
            {
                return;
            }
            if (_szData >= m_Size2Allocator.size())
            {
                m_szSystemAllocate -= _szData;
                ::free(_pPtr);
            }
            else
            {
                m_Allocators[m_Size2Allocator[_szData]].deallocate(_pPtr);
            }
        }

        size_t usedBytes() const
        {
            size_t szUse = 0;
            for (const auto& allo : m_Allocators)
            {
                szUse += allo.usedBytes();
            }
            szUse += m_szSystemAllocate;
            return szUse;
        }

        size_t totalBytes() const
        {
            size_t szUse = 0;
            for (const auto& allo : m_Allocators)
            {
                szUse += allo.totalBytes();
            }
            szUse += m_szSystemAllocate;
            return szUse;
        }

    private:
        void reset(size_t _szPageSize, size_t _szAlgin, const std::vector<size_t>& _vecBlockSize)
        {
            m_Allocators.resize(_vecBlockSize.size());
            m_Size2Allocator.resize(_vecBlockSize.back() + 1);

            size_t index2Map = 0;
            size_t allocSize = 0;
            for (size_t index = 0, size = _vecBlockSize.size(); index < size; ++index)
            {
                allocSize = _vecBlockSize[index];
                m_Allocators[index].reset(allocSize, _szPageSize, _szAlgin);
                while (index2Map <= allocSize)
                {
                    m_Size2Allocator[index2Map++] = index;
                }
            }
        }


    private:
        //分页大小\0
        static const size_t MEMORY_POOL_PAGE_SIZE = 1024 * 32;
        static const size_t MEMORY_POOL_PAGE_ALGIN = 4;

        std::vector<CAllocator>     m_Allocators;               //分配器\0
        std::vector<size_t>         m_Size2Allocator;           //内容容量分配器映射表\0
        size_t                      m_szSystemAllocate = 0;     //使用系统分配器所分配的内存(直接malloc和free)\0
        std::string                 m_strPoolTag;
    };


}
