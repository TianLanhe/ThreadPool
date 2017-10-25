#include "WinThread.h"
#include "WinThreadPool.h"
#include "../../include/Task.h"

Thread* GGetWinThread(ThreadPool* pool) {
	return new WinThread(pool);
}

WinThread::WinThread(ThreadPool* pool) :m_threadPool(pool), m_bRun(false), m_hThread(NULL) {
	Create();
}

WinThread::~WinThread() {
	Destroy();
}

ThreadPool* WinThread::GetThreadPool() {
	return m_threadPool;
}

Status WinThread::Create() {
	CHECK_ERROR(!m_hThread && !m_bRun);
	m_hThread = CreateThread(NULL, 0, _ThreadProc, this, CREATE_SUSPENDED, NULL);
	return m_hThread != NULL ? OK : ER;
}

Status WinThread::Destroy() {
	CHECK_ERROR(m_hThread);

	BEFORE_CHECK_RESULT();
	if (m_bRun) {
		CHECK_RESULT(Terminate());
		m_bRun = false;
	}
	BOOL r = ::CloseHandle(m_hThread);
	m_hThread = NULL;
	CHECK_ERROR(r);
	AFTER_CHECK_RESULT();
}

Status WinThread::Start() {
	CHECK_ERROR(m_hThread && !m_bRun);
	BOOL r = ::ResumeThread(m_hThread);
	if (r != -1)
		m_bRun = true;
	return r != -1 ? OK : ER;
}

Status WinThread::Wait() {
	CHECK_ERROR(m_hThread);
	WinThreadPool* pool = dynamic_cast<WinThreadPool*>(GetThreadPool());
	CHECK_ERROR(pool);
	BEFORE_CHECK_RESULT();
	CHECK_RESULT(pool->Request());
	AFTER_CHECK_RESULT();
}

Status WinThread::Terminate() {
	CHECK_ERROR(m_hThread && m_bRun);
	BOOL r = ::TerminateThread(m_hThread, 0);
	return r ? OK : ER;
}

bool WinThread::IsRunning() {
	return m_bRun;
}

DWORD WINAPI WinThread::_ThreadProc(LPVOID lpParameter) {
	WinThread* ptrThis = (WinThread*)lpParameter;
	WinThreadPool* pool;
	Task* task;

	pool = dynamic_cast<WinThreadPool*>(ptrThis->GetThreadPool());
	if (pool == NULL)
		return 1;

	while (1) {
		task = pool->GetTask();
		if (task) {
			task->Run();
			delete task;
			task = nullptr;
		}
		else {
			pool->Release();
			ptrThis->m_bRun = false;
			::SuspendThread(ptrThis->m_hThread);
		}
	}
	return 0;
}
