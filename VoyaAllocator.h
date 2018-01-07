
#pragma once

#include <cstddef>
#include <cstdint>
#include "VoyaUtil/VoyaAssert.h"

#ifdef MEMORY_POOL_DEBUG
#include <unordered_map>
#endif

namespace voya
{
    class CAllocatorHelper
    {
    public:
        static size_t getAlignInfo(size_t _dataSize, size_t _alignSize)
        {
            return (((_dataSize)+((_alignSize)-1)) & ~((_alignSize)-1));
        }
    };
    
    
    class CAllocator
    {
    public:
        static const size_t DEFAULT_ALGIN = 4;

        class CBlockHeader
        {
        public:
            CBlockHeader*   m_pNext = nullptr;
        };

        class CPageHeader
        {
        public:
            inline CBlockHeader*   blocks()
            {
                return reinterpret_cast<CBlockHeader*>(this + 1);   //数据前端是下一页的指针, this+1偏移才是真正的可用内存\0
            }

            CPageHeader*    m_pNext = nullptr;
        };

        CAllocator()
        {

        }

        CAllocator(size_t _szDataSize, size_t _szPageSize, size_t _szAlignSize = DEFAULT_ALGIN)
        {
            reset(_szDataSize, _szPageSize, _szAlignSize);
        }

        ~CAllocator()
        {
            freeAll();
        }

        void  reset(size_t _szDataSize, size_t _szPageSize, size_t _szAlignSize)
        {
            freeAll();

            m_szPageSize = _szPageSize + sizeof(CPageHeader);

            //内存对齐成2^n次方\0
            m_szBlockSize = _szDataSize > sizeof(CBlockHeader) ? _szDataSize : sizeof(CBlockHeader);
            m_szAlignSize = CAllocatorHelper::getAlignInfo(m_szBlockSize, _szAlignSize) - m_szBlockSize;
            m_szBlockSize += m_szAlignSize;

            m_szBlockPerPage = (m_szPageSize - sizeof(CPageHeader)) / m_szBlockSize;

        }

        void  freeAll()
        {
            if (m_szFreeBlockCount != m_szBlockCount)
            {
                voya_log_warning("POOL[PageSize:%d, DataSize:%d] leak:%d, created %d objects, %d objects are not return", m_szPageSize, m_szBlockSize, usedBytes(), m_szBlockCount, m_szBlockCount - m_szFreeBlockCount);

#ifdef MEMORY_POOL_DEBUG
                for (auto kv : m_debugMap)
                {
                    if (nullptr != kv.second)
                    {
                        voya_log_warning("%s : 0x%08x", kv.second, kv.first);
                    }
                    else
                    {
                        voya_log_warning("not operator new create: 0x%08x", kv.first);
                    }
                }
#endif

            }

            for (CPageHeader* pPageHeader = m_pPageList; pPageHeader != nullptr; )
            {
                CPageHeader* pPageNext = pPageHeader->m_pNext;

#ifdef MEMORY_POOL_DEBUG
                fillFreePage(pPageHeader);
#endif

                ::free(pPageHeader);

                pPageHeader = pPageNext;
            }

            m_pPageList = nullptr;
            m_pFreeBlockList = nullptr;
            m_szBlockCount = 0;
            m_szFreeBlockCount = 0;
            m_szPageCount = 0;
        }

#ifdef MEMORY_POOL_DEBUG
        void* allocate(const char* _pszClassName = nullptr)
#else
        void* allocate()
#endif
        {
            if (0 == m_szFreeBlockCount)
            {
                //没有闲置的Block了\0
                CPageHeader* pPageHeader = reinterpret_cast<CPageHeader*>(::malloc(m_szPageSize));
                CBlockHeader* pBlockHeader = pPageHeader->blocks();
                CBlockHeader* pBlockNext = nullptr;
                for (int iIndex = 0; iIndex < m_szBlockPerPage; ++iIndex)
                {
                    pBlockNext = (iIndex != (m_szBlockPerPage - 1) ? reinterpret_cast<CBlockHeader*>(reinterpret_cast<int8_t*>(pBlockHeader)+m_szBlockSize) : nullptr);
#ifdef MEMORY_POOL_DEBUG
                    fillAllocBlock(pBlockHeader);
#endif
                    pBlockHeader->m_pNext = pBlockNext;
                    pBlockHeader = pBlockNext;
                }
                m_szPageCount++;
                m_szBlockCount += m_szBlockPerPage;
                m_szFreeBlockCount += m_szBlockPerPage;

                pPageHeader->m_pNext = m_pPageList;
                m_pPageList = pPageHeader;
                m_pFreeBlockList = pPageHeader->blocks();
            }

            CBlockHeader* pReturn = m_pFreeBlockList;
            m_pFreeBlockList = m_pFreeBlockList->m_pNext;
            m_szFreeBlockCount--;
#ifdef MEMORY_POOL_DEBUG
            fillAllocBlock(pReturn);
            if (nullptr != _pszClassName)
            {
                m_debugMap.emplace(pReturn, _pszClassName);
            }
            else
            {
                m_debugMap.emplace(pReturn, nullptr);
            }
#endif
            return pReturn;
        }

        void  deallocate(void* _pBlockHeader)
        {
            if (nullptr != _pBlockHeader)
            {
                CBlockHeader* pBlockHead = reinterpret_cast<CBlockHeader*>(_pBlockHeader);
#ifdef MEMORY_POOL_DEBUG
                fillFreeBlock(pBlockHead);
                auto iter = m_debugMap.find(pBlockHead);
                if (iter == m_debugMap.end())
                {
                    voya_log_warning("return a object not borrow");
                }
                else
                {
                    m_debugMap.erase(iter);
                }
#endif
                pBlockHead->m_pNext = m_pFreeBlockList;
                m_pFreeBlockList = pBlockHead;
                m_szFreeBlockCount++;
            }
        }

        size_t usedBytes() const
        {
            return m_szBlockSize * (m_szBlockCount - m_szFreeBlockCount);
        }

        size_t totalBytes() const
        {
            return m_szBlockSize * m_szBlockCount;
        }

#ifdef MEMORY_POOL_DEBUG
    private:
        // debug patterns
        static const uint8_t PATTERN_ALIGN  = 0xFC;
        static const uint8_t PATTERN_ALLOC  = 0xFD;
        static const uint8_t PATTERN_FREE   = 0xFE;

        void fillFreeBlock(CBlockHeader* _pBlockHeader)
        {
            uint8_t* pAddr = reinterpret_cast<uint8_t*>(_pBlockHeader);
            std::memset(pAddr, PATTERN_FREE, m_szBlockSize - m_szAlignSize);
            std::memset(pAddr + m_szBlockSize - m_szAlignSize, PATTERN_ALIGN, m_szAlignSize);
        }

        void fillFreePage(CPageHeader* _pPageHeader)
        {
            uint8_t* pAddr = reinterpret_cast<uint8_t*>(_pPageHeader);
            std::memset(pAddr, PATTERN_FREE, m_szPageSize);
        }

        void fillAllocBlock(CBlockHeader* _pBlockHeader)
        {
            uint8_t* pAddr = reinterpret_cast<uint8_t*>(_pBlockHeader);
            std::memset(pAddr, PATTERN_ALLOC, m_szBlockSize - m_szAlignSize);
            std::memset(pAddr + m_szBlockSize - m_szAlignSize, PATTERN_ALIGN, m_szAlignSize);
        }

#endif


    private:

        //初始化就确定的内容\0
        size_t          m_szPageSize        = 0;            //分页大小\0
        size_t          m_szBlockSize       = 0;            //Block真实大小(Next指针+真实数据+对齐扩展)\0
        size_t          m_szAlignSize       = 0;            //为了内存对齐扩展的大小\0
        size_t          m_szBlockPerPage    = 0;            //每页的可用Block数目\0

        //运行中变化的内容\0
        size_t          m_szPageCount       = 0;            //总共生成的Page数目\0
        size_t          m_szBlockCount      = 0;            //总共生成的Block数目\0
        size_t          m_szFreeBlockCount  = 0;            //闲置的Block数目\0

        CPageHeader*    m_pPageList         = nullptr;
        CBlockHeader*   m_pFreeBlockList    = nullptr;

#ifdef MEMORY_POOL_DEBUG
        std::unordered_map<void*, const char*> m_debugMap;
#endif

    };


}

