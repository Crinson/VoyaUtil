
//#include <windows.h>

#include "VoyaUtil/VoyaUtilTest.h"
#include "VoyaUtil/VoyaAssert.h"

#include "VoyaUtil/VoyaSharePtr.h"
#include "VoyaUtil/VoyaAllocator.h"
#include "VoyaUtil/VoyaObjectPool.h"

namespace voya
{
#ifdef _WIN32
    
#else
#include <time.h>
    unsigned long GetTickCount()
    {
        struct timespec ts;
        clock_gettime(_CLOCK_MONOTONIC, &ts);
        return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
    }
#endif
    
    //////////////////////////////////////////////////////////////////////////
    class CTestClass
    {
    public:
        CTestClass(size_t _i, bool _bLog = true)
            : m_i(_i)
            , m_bLog(_bLog)
        {
            if (m_bLog)
            {
                voya_log("CTestClass Construct %d", m_i);
            }
        }

        ~CTestClass()
        {
            if (m_bLog)
            {
                voya_log("CTestClass Destruct %d", m_i);
            }
            m_i = 0;
        }

        void show()
        {
            voya_log("CTestClass show %d", m_i);
        }

    private:
        size_t m_i = 0;
        bool   m_bLog = false;
        char   m_tmp[8];
    };


    //////////////////////////////////////////////////////////////////////////

#define CHECK_POOL(TAG) if (0 != CSTLPoolInstance::instance()->usedBytes()){voya_log_warning("CSTLPoolInstance is not empty , %s", TAG);}

#define RUN_TEST(_FUNCTION) \
    voya_log("\ntest %s start=============================", #_FUNCTION);\
    if(test##_FUNCTION())\
    {\
        voya_log("test %s success", #_FUNCTION);\
    }\
    else\
    {\
        voya_log_warning("test %s failed", #_FUNCTION);\
    }\
    CHECK_POOL("after "#_FUNCTION);


    bool CUtilTest::testAll()
    {
        RUN_TEST(SharePtr);
        RUN_TEST(Allocator);
        RUN_TEST(ObjectPool);
        testPerformance();
        return true;
    }


    //////////////////////////////////////////////////////////////////////////
    void CUtilTest::enter()
    {
        CHECK_POOL("before test");
    }

    void CUtilTest::exit()
    {
        CHECK_POOL("after test");
    }
    //////////////////////////////////////////////////////////////////////////

    bool CUtilTest::testSharePtr()
    {
        //////////////////////////////////////////////////////////////////////////
        //Construct test
        shared_ptr<CTestClass>  funcConstruct(new CTestClass(1));
        if (1 != funcConstruct.use_count())
        {
            return false;
        }

        shared_ptr<CTestClass>  funcCopyConstruct(funcConstruct);
        if (2 != funcConstruct.use_count() || 2 != funcCopyConstruct.use_count())
        {
            return false;
        }

        shared_ptr<CTestClass>  funcCopyConstruct2 = funcConstruct;
        if (3 != funcConstruct.use_count() || 3 != funcCopyConstruct2.use_count())
        {
            return false;
        }

        shared_ptr<CTestClass>  funcMoveConstruct(std::move(shared_ptr<CTestClass>(new CTestClass(2))));
        if (1 != funcMoveConstruct.use_count())
        {
            return false;
        }

        shared_ptr<CTestClass>  funcSetConstruct;
        if (0 != funcSetConstruct.use_count())
        {
            return false;
        }

        funcSetConstruct = funcConstruct;
        if (4 != funcConstruct.use_count() || 4 != funcSetConstruct.use_count())
        {
            return false;
        }

        shared_ptr<CTestClass> funcDeletorConstruct(new CTestClass(3), [](CTestClass* _p)
        {
            _p->show();
            delete _p;
        });

        funcDeletorConstruct = funcSetConstruct;
        if (5 != funcConstruct.use_count() || 5 != funcSetConstruct.use_count() || 5 != funcDeletorConstruct.use_count())
        {
            return false;
        }


        //////////////////////////////////////////////////////////////////////////
        //reset test
        funcCopyConstruct2.reset();
        if (0 != funcCopyConstruct2.use_count() || 4 != funcSetConstruct.use_count() || 4 != funcDeletorConstruct.use_count())
        {
            return false;
        }

        funcCopyConstruct2.reset(new CTestClass(4));
        if (1 != funcCopyConstruct2.use_count())
        {
            return false;
        }

        //////////////////////////////////////////////////////////////////////////
        //swap test
        funcCopyConstruct2.swap(funcConstruct);
        if (4 != funcCopyConstruct2.use_count() || 1 != funcConstruct.use_count() || 4 != funcDeletorConstruct.use_count())
        {
            return false;
        }

        //////////////////////////////////////////////////////////////////////////
        //==
        if (funcCopyConstruct2 != funcCopyConstruct)
        {
            return false;
        }

        return true;
    }

    bool CUtilTest::testAllocator()
    {
        const int MAX_TEST_TIMES = 100;
        const int BLOCK_PER_PAGE = 4;
        const int CLASS_SIZE = sizeof(CTestClass);
        using ShareTest = shared_ptr<CTestClass>;

        CAllocator allo(CLASS_SIZE, BLOCK_PER_PAGE * CLASS_SIZE);
        std::list<ShareTest> list;

        for (int i = 1; i <= MAX_TEST_TIMES; i++)
        {
            list.push_back(ShareTest(new(allo.allocate())CTestClass(i), [&allo](CTestClass* _p){ _p->~CTestClass(); allo.deallocate(_p); }));
        }
        if (MAX_TEST_TIMES * CLASS_SIZE != allo.usedBytes())
        {
            return false;
        }
        list.clear();

        {
            for (int i = 1; i <= 5; i++)
            {
                list.push_back(ShareTest(new(allo.allocate())CTestClass(i), [&allo](CTestClass* _p){ _p->~CTestClass(); allo.deallocate(_p); }));
            }

            if (5 * CLASS_SIZE != allo.usedBytes())
            {
                return false;
            }
            list.clear();
        }

        if (0 != allo.usedBytes())
        {
            return false;
        }
        
        for (int i = 1; i <= 3; i++)
        {
            list.push_back(ShareTest(new(allo.allocate())CTestClass(i), [&allo](CTestClass* _p){ _p->~CTestClass(); allo.deallocate(_p); }));
        }
        if (3 * CLASS_SIZE != allo.usedBytes())
        {
            return false;
        }

        {
            for (int i = 4; i <= 11; i++)
            {
                list.push_back(ShareTest(new(allo.allocate())CTestClass(i), [&allo](CTestClass* _p){ _p->~CTestClass(); allo.deallocate(_p); }));
            }
            if (11 * CLASS_SIZE != allo.usedBytes())
            {
                return false;
            }
            for (int i = 1; i <= 5; i++)
            {
                list.pop_front();
            }
        }

        if (6 * CLASS_SIZE != allo.usedBytes())
        {
            return false;
        }
        list.clear();

        for (int i = 1; i <= MAX_TEST_TIMES; i++)
        {
            list.push_back(ShareTest(new(allo.allocate())CTestClass(i), [&allo](CTestClass* _p){ _p->~CTestClass(); allo.deallocate(_p); }));
        }

        int iIndex = 0;
        for (auto sp : list)
        {
            iIndex++;
            if (iIndex % 1500 == 0)
            {
                sp->show();
            }
        }
        list.clear();

        return true;
    }

    bool CUtilTest::testObjectPool()
    {
        const int BLOCK_PER_PAGE = 40;
        const int CLASS_SIZE = 1;
        using ShareTest = shared_ptr<CTestClass>;

        //origin_object
        {
            auto* pPool = CObjectPoolSingleton<CTestClass, BLOCK_PER_PAGE>::instance();
            const int MAX_TEST_TIMES = 1000;
            std::list<ShareTest> list;

            for (int i = 1; i <= MAX_TEST_TIMES; i++)
            {
                list.push_back(ShareTest(pPool->borrowObject(i), [pPool](CTestClass* _p){pPool->returnObject(_p); }));
            }
            if (MAX_TEST_TIMES * CLASS_SIZE != pPool->getUseCount())
            {
                return false;
            }
            list.clear();

            {
                for (int i = 1; i <= 5; i++)
                {
                    list.push_back(ShareTest(pPool->borrowObject(i), [pPool](CTestClass* _p){pPool->returnObject(_p); }));
                }

                if (5 * CLASS_SIZE != pPool->getUseCount())
                {
                    return false;
                }
                list.clear();
            }

            if (0 != pPool->getUseCount())
            {
                return false;
            }

            for (int i = 1; i <= 3; i++)
            {
                list.push_back(ShareTest(pPool->borrowObject(i), [pPool](CTestClass* _p){pPool->returnObject(_p); }));
            }
            if (3 * CLASS_SIZE != pPool->getUseCount())
            {
                return false;
            }

            {
                for (int i = 4; i <= 11; i++)
                {
                    list.push_back(ShareTest(pPool->borrowObject(i), [pPool](CTestClass* _p){pPool->returnObject(_p); }));
                }
                if (11 * CLASS_SIZE != pPool->getUseCount())
                {
                    return false;
                }
                for (int i = 1; i <= 5; i++)
                {
                    list.pop_front();
                }
            }

            if (6 * CLASS_SIZE != pPool->getUseCount())
            {
                return false;
            }
            list.clear();

            for (int i = 1; i <= MAX_TEST_TIMES; i++)
            {
                list.push_back(ShareTest(pPool->borrowObject(i), [pPool](CTestClass* _p){pPool->returnObject(_p); }));
            }

            int iIndex = 0;
            for (auto sp : list)
            {
                iIndex++;
                if (iIndex % 1500 == 0)
                {
                    sp->show();
                }
            }
            list.clear();
            CObjectPoolSingleton<CTestClass, BLOCK_PER_PAGE>::destroy();
        }

        //shapre_object
        {
            auto* pPool = CObjectPoolSingleton<CTestClass, BLOCK_PER_PAGE>::instance();
            const int MAX_TEST_TIMES = 1000;
            std::list<ShareTest> list;

            for (int i = 1; i <= MAX_TEST_TIMES; i++)
            {
                list.push_back(pPool->borrowSharedObject(i));
            }
            if (MAX_TEST_TIMES * CLASS_SIZE != pPool->getUseCount())
            {
                return false;
            }
            list.clear();

            {
                for (int i = 1; i <= 5; i++)
                {
                    list.push_back(pPool->borrowSharedObject(i));
                }

                if (5 * CLASS_SIZE != pPool->getUseCount())
                {
                    return false;
                }
                list.clear();
            }

            if (0 != pPool->getUseCount())
            {
                return false;
            }

            for (int i = 1; i <= 3; i++)
            {
                list.push_back(pPool->borrowSharedObject(i));
            }
            if (3 * CLASS_SIZE != pPool->getUseCount())
            {
                return false;
            }

            {
                for (int i = 4; i <= 11; i++)
                {
                    list.push_back(pPool->borrowSharedObject(i));
                }
                if (11 * CLASS_SIZE != pPool->getUseCount())
                {
                    return false;
                }
                for (int i = 1; i <= 5; i++)
                {
                    list.pop_front();
                }
            }

            if (6 * CLASS_SIZE != pPool->getUseCount())
            {
                return false;
            }
            list.clear();

            for (int i = 1; i <= MAX_TEST_TIMES; i++)
            {
                list.push_back(pPool->borrowSharedObject(i));
            }

            int iIndex = 0;
            for (auto sp : list)
            {
                iIndex++;
                if (iIndex % 1500 == 0)
                {
                    sp->show();
                }
            }
            list.clear();
            CObjectPoolSingleton<CTestClass, BLOCK_PER_PAGE>::destroy();
        }

        return true;
    }

    void CUtilTest::testPerformance()
    {
        const int MAX_TEST_TIMES = 10000;
        int start = 0;
        int end = 0;
        auto* pPool = CObjectPoolSingleton<CTestClass, 1000>::instance();

        using OriginType = CTestClass;
        using SharedType = voya::shared_ptr<CTestClass>;
        voya::list<OriginType*> listOrigin;
        voya::list<SharedType> listShared;

        voya_log("=====start %s", "pool");
        start = GetTickCount();
        for (int iIndex = 1; iIndex <= MAX_TEST_TIMES; ++iIndex)
        {
            listOrigin.push_back(pPool->borrowObject(iIndex, false));
        }
        end = GetTickCount();
        voya_log("=====use time %d", end - start);
        for (OriginType* p : listOrigin)
        {
            pPool->returnObject(p);
        }
        listOrigin.clear();

        voya_log("=====start %s", "pool2");
        start = GetTickCount();
        for (int iIndex = 1; iIndex <= MAX_TEST_TIMES; ++iIndex)
        {
            listOrigin.push_back(pPool->borrowObject(iIndex, false));
        }
        end = GetTickCount();
        voya_log("=====use time %d", end - start);
        for (OriginType* p : listOrigin)
        {
            pPool->returnObject(p);
        }
        listOrigin.clear();

        voya_log("=====start %s", "origin");
        start = GetTickCount();
        for (int iIndex = 1; iIndex <= MAX_TEST_TIMES; ++iIndex)
        {
            listOrigin.push_back(new OriginType(iIndex, false));
        }
        end = GetTickCount();
        voya_log("=====use time %d", end - start);
        for (OriginType* p : listOrigin)
        {
            delete p;
        }
        listOrigin.clear();


    }

}
