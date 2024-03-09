#pragma once

#include<unistd.h>
#include<sys/syscall.h>

namespace CurrentThread
{
    extern __thread int t_cachedTid;

    void cacheTid();

    inline int tid()
    {
        if(__builtin_expect(t_cachedTid == 0, 0))   //编译器优化，第二个零表示前面表达式为假的概率很大
        {
            cacheTid();
        }
        return t_cachedTid;
    }
}