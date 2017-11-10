#include "LinuxThread.h"
#include "LinuxThreadPool.h"
#include "../../include/Task.h"

Thread* GGetLinuxThread(ThreadPool* pool) {
    return new LinuxThread(pool);
}

LinuxThread::LinuxThread(ThreadPool* pool) :m_threadPool(pool), m_bRun(false),
    m_theadId(NULL),
    m_startAndStopMutex(NULL), m_runAndWaitMutex(NULL) {
    Create();
}

LinuxThread::~LinuxThread() {
    Destroy();
}

ThreadPool* LinuxThread::GetThreadPool() {
    return m_threadPool;
}

Status LinuxThread::Create() {
    CHECK_ERROR(!m_theadId && !m_bRun);
    
    if(m_startAndStopMutex){
        pthread_mutex_destroy(m_startAndStopMutex);
        free(m_startAndStopMutex);
        delete m_startAndStopMutex = NULL;
    }
    if(m_runAndWaitMutex){
        pthread_mutex_destroy(m_runAndWaitMutex);
        free(m_runAndWaitMutex);
        m_runAndWaitMutex = NULL;
    }
    
    m_startAndStopMutex = new pthread_mutex_t;
    pthread_mutex_init(m_startAndStopMutex,NULL);
    
    m_runAndWaitMutex = new pthread_mutex_t;
    pthread_mutex_init(m_runAndWaitMutex,NULL);
    
    pthread_mutex_lock(m_startAndStopMutex);
    
    m_theadId = new pthread_t;
    int ret = pthread_create(m_theadId,NULL,_ThreadProc,(void*)this);
    
    return ret == 0 ? OK : ER;
}

Status LinuxThread::Destroy() {
    CHECK_ERROR(m_theadId);
    
    BEFORE_CHECK_RESULT();
    pthread_cancel(*m_theadId);
    if (!m_bRun) {
        _assume();
    }
    pthread_join(*m_theadId);
    delete m_theadId;
    m_theadId = NULL;
    AFTER_CHECK_RESULT();
}

Status LinuxThread::Start() {
    CHECK_ERROR(m_theadId != PTHREAD_NULL && !m_bRun);
    _assume();
    _start();
    return OK;
}

Status LinuxThread::Wait() {
    CHECK_ERROR(m_theadId && !m_bRun);
    _lock(m_runMutex);
    while(m_bRun)
        _waitForCondition(m_cond,m_runMutex);
    _unlock(m_runMutex);
    return OK;
}

Status LinuxThread::Terminate() {
    _lock(m_terminateMutex);
    m_bTerminate = true;
    _unlock(m_terminateMutex);
    void* val;
    int ret = pthread_join(m_theadId,&val);
    return ret == 0 && (*(long*)val) == 0 ? OK : ER;
}

bool LinuxThread::_shouldTerminate(){
    _lock(m_terminateMutex);
    bool terminate = m_bTerminate;
    _unlock(m_terminateMutex);
    return stop;
}

bool LinuxThread::IsRunning() {
    _lock(m_runMutex);
    bool run = m_bRun;
    _unlock(m_runMutex);
    return run;
}

bool LinuxThread::IsEqual(const Thread& thread){
    Thread* ptr = &thread;
    LinuxThread& linuxthread = thread;
    return dynamic_cast<LinuxThread*>(ptr) && pthread_equal(m_theadId,linuxthread.m_theadId);
}

void* LinuxThread::_ThreadProc(void* ptrVoid) {
    LinuxThread* ptrThis = (LinuxThread*)ptrVoid;
    LinuxThreadPool* pool;
    Task* task;
    
    pool = dynamic_cast<LinuxThreadPool*>(ptrThis->GetThreadPool());
    if (pool == NULL)
        return (void*)1;
    
    while (1) {
        if(ptrThis->_shouldStop())
            break;
        
        task = pool->GetTask();
        if (task) {
            task->Run();
            delete task;
            task = nullptr;
        }
        else {
            ptrThis->_suspend();
        }
    }
    return (void*)0;
}
