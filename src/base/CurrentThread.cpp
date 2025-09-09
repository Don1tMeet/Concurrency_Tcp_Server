#include "CurrentThread.h"
#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
namespace CurrentThread
{
    // __thread为线程局部变量，每个线程都有一个副本
    __thread int t_cachedTid = 0;  // 存储当前线程id
    __thread char t_formattedTid[32];  // 存储当前线程id的字符串格式
    __thread int t_formattedTidLength;  // t_formattedTid的长度

    pid_t gettid() {
        // 不使用std::this_thread::get_id()，它好像并不是唯一的
        return static_cast<int>(syscall(SYS_gettid));  // 使用系统调用gettid获取当前线程id
    }
    void CacheTid(){  // 缓存当前线程id
        if (t_cachedTid == 0){  // ==0，说明还没有缓存
            t_cachedTid = gettid();
            t_formattedTidLength = snprintf(t_formattedTid, sizeof(t_formattedTid), "%5d ", t_cachedTid);
        }
    }
}
    
