
#pragma once

namespace voya
{
    //禁止派生类拷贝类\0
    class CNoCopyAble
    {
    protected:
        CNoCopyAble() = default;
        virtual ~CNoCopyAble() = default;

        //禁用拷贝构造\0
        CNoCopyAble(const CNoCopyAble&) = delete;
        //禁用赋值构造\0
        CNoCopyAble& operator=(const CNoCopyAble&) = delete;

    };

}