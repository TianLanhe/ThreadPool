#include "LinuxThread.h"
#include "LinuxThreadPool.h"
#include "../../include/Task.h"

Thread* GGetLinuxThread(ThreadPool* pool) {
    return new LinuxThread(pool);
}

LinuxThread::LinuxThread(ThreadPool* pool) :
	m_threadPool(pool), m_bRun(false), m_bTerminate(false), 
    m_threadId(NULL),
    m_runMutex(NULL), m_terminateMutex(NULL), m_cond(NULL) {
    Create();
}

LinuxThread::~LinuxThread() {
    Destroy();
}

ThreadPool* LinuxThread::GetThreadPool() {
    return m_threadPool;
}

Status LinuxThread::Create() {
    CHECK_ERROR(!m_threadId);
    
    BEFORE_CHECK_RESULT();

    if(m_cond)
        CHECK_RESULT(_destroyCond(m_cond));
    CHECK_RESULT(_initCond(m_cond));

    if(m_runMutex)
    	CHECK_RESULT(_destroyMutex(m_runMutex));
    CHECK_RESULT(_initMutex(m_runMutex));

    if(m_terminateMutex)
    	CHECK_RESULT(_destroyMutex(m_terminateMutex));
    CHECK_RESULT(_initMutex(m_terminateMutex));

    m_bRun = false;
    m_bTerminate = false;
    
    m_threadId = new pthread_t;
    int ret = pthread_create(m_threadId,NULL,_ThreadProc,(void*)this);
    
    return ret == 0 ? OK : ER;
}

Status LinuxThread::Destroy() {
    CHECK_ERROR(m_threadId);
    
    BEFORE_CHECK_RESULT();
    CHECK_RESULT(Terminate());
    delete m_threadId;
    m_threadId = NULL;

    m_bRun = false;
    m_bTerminate = false;

    CHECK_RESULT(_destroyMutex(m_terminateMutex));
    CHECK_RESULT(_destroyMutex(m_runMutex));
    CHECK_RESULT(_destroyCond(m_cond));

    AFTER_CHECK_RESULT();
}

Status LinuxThread::Start() {
    CHECK_ERROR(m_threadId && !m_bRun);
    _assume();
    return OK;
}

Status LinuxThread::Wait() {
    CHECK_ERROR(m_threadId && m_bRun);
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

    if(!IsRunning())
    	_assume();

    void* val;
    int ret = pthread_join(*m_threadId,&val);
    return ret == 0 && (*(Status*)val) == OK ? OK : ER;
}

bool LinuxThread::_shouldTerminate(){
	bool terminate;
    _lock(m_terminateMutex);
    terminate = m_bTerminate;
    _unlock(m_terminateMutex);
    return terminate;
}

void LinuxThread::_suspend(){
	_lock(m_runMutex);
	if(m_bRun){
		m_bRun = false;
		_unlock(m_runMutex);
		_notifyAll(m_cond);

		_lock(m_runMutex);
		while(!m_bRun)
			_waitForCondition(m_cond,m_runMutex);
		_unlock(m_runMutex);
	}else{
		_lock(m_runMutex);
		while(!m_bRun)
			_waitForCondition(m_cond,m_runMutex);
		_unlock(m_runMutex);
	}
}

void LinuxThread::_assume(){
	_lock(m_runMutex);
	if(!m_bRun){
		m_bRun = true;
		_unlock(m_runMutex);
		_notifyAll(m_cond);
	}else{
		_unlock(m_runMutex);
	}
}

bool LinuxThread::IsRunning() {
	bool run;
    _lock(m_runMutex);
    run = m_bRun;
    _unlock(m_runMutex);
    return run;
}

bool LinuxThread::IsEqual(const Thread& thread){
    const Thread* ptr = &thread;
    const LinuxThread* linuxthread = dynamic_cast<const LinuxThread*>(ptr);
    return linuxthread && pthread_equal(*m_threadId,*(linuxthread->m_threadId));
}

void* LinuxThread::_ThreadProc(void* ptrVoid) {
    LinuxThread* ptrThis = (LinuxThread*)ptrVoid;
    LinuxThreadPool* pool;
    Task* task;
    
    pool = dynamic_cast<LinuxThreadPool*>(ptrThis->GetThreadPool());
    if (pool == NULL)
        return (void*)ER;

    ptrThis->_suspend();
    
    while (1) {
        if(ptrThis->_shouldTerminate())
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
    return (void*)OK;
}
