#ifndef LINUX_THREAD_H
#define LINUX_THREAD_H

#include "../../include/Thread.h"
#include <pthread.h>

class LinuxThread : public Thread
{
public:
    explicit LinuxThread(ThreadPool*);
    ~LinuxThread();
    
    ThreadPool *GetThreadPool();
    
    Status Create();
    Status Destroy();
    
    Status Start();
    Status Wait();
    Status Terminate();
    
    bool IsRunning();
    
    bool IsEqual(const Thread&);
    
    void _shouldTerminate();
    void _suspend();
    void _assume();
    
private:
    static void* _ThreadProc(void*);
    
    void _lock(pthread_mutex_t* mutex){
        pthread_mutex_lock(mutex);
    }
    
    void _unlock(pthread_mutex_t* mutex){
        pthread_mutex_unlock(mutex);
    }
    
    void _waitForCondition(pthread_mutex_t* mutex,pthread_cond_t* cond){
        pthread_cond_wait(cond,mutex);
    }
    
    ThreadPool* m_threadPool;
    pthread_t* m_threadId;
    
    bool m_bRun;
    pthread_mutex_t* m_runMutex;
    pthread_cond_t* m_cond;
    
    bool m_bTerminate;
    pthread_mutex_t* m_terminateMutex;
};

Thread* GGetLinuxThread(ThreadPool* pool);

#endif // !LINUX_THREAD_H
