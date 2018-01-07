
#pragma once

namespace voya
{
    //////////////////////////////////////////////////////////////////////////
    //测试类
    class CUtilTest
    {
    public:
        static void enter();
        static void exit();

        static bool testAll();

        static bool testSharePtr();
        static bool testAllocator();
        static bool testObjectPool();

        static void testPerformance();
    };


}
