#pragma once

#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
namespace CurrentThread
{
    extern __thread int t_cachedTid;
    extern __thread char t_formattedTid[32];
    extern __thread int t_formattedTidLength;

    void CacheTid();  // 缓存当前线程id

    pid_t gettid();  // 获取当前线程id

    inline int tid() {  // 获取缓存的id
        if (__builtin_expect(t_cachedTid == 0, 0)) {
            CacheTid();
        }
        return t_cachedTid;
    }


    inline const char *tidString() { return t_formattedTid; }
    inline int tidStringLength() { return t_formattedTidLength; }
}