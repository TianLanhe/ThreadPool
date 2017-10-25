#include "WinThreadPool.h"
#include "WinThread.h"
#include "../../include/Task.h"

using namespace std;

WinThreadPool::WinThreadPool(int threadsNum, int maxThreadsNum) :m_threadsNum(threadsNum), m_maxThreadsNum(maxThreadsNum), m_bRun(false), m_workingThreadsNum(0) {
	if (m_maxThreadsNum < m_threadsNum)
		m_maxThreadsNum = m_threadsNum;

	InitializeCriticalSection(&m_cs);
	m_hSemaphore = CreateSemaphore(NULL, 0, m_maxThreadsNum, NULL);

	for (int i = 0; i < m_threadsNum; ++i)
		m_threads.push_back(GGetWinThread(this));
}

WinThreadPool::~WinThreadPool() {
	if (!m_threads.empty()) {
		for (vector<Thread*>::size_type i = 0; i < m_threads.size(); ++i)
			delete m_threads[i];
		m_threads.clear();
	}
	while (!m_tasks.empty()) {
		Task *task = m_tasks.front();
		m_tasks.pop();
		delete task;
	}
	DeleteCriticalSection(&m_cs);
	CloseHandle(m_hSemaphore);
}

Status WinThreadPool::StartTasks() {
	CHECK_ERROR(!m_bRun && !m_workingThreadsNum);

	m_bRun = true;
	BEFORE_CHECK_RESULT();
	for (vector<Thread>::size_type i = 0; i < m_threads.size(); ++i) {
		CHECK_RESULT(m_threads[i]->Start());
		++m_workingThreadsNum;
	}
	return OK;
}

Status WinThreadPool::WaitForThreads() {
	CHECK_ERROR(m_bRun && m_workingThreadsNum);

	BEFORE_CHECK_RESULT();
	for (vector<Thread>::size_type i = 0; i < m_threads.size(); ++i) {
		CHECK_RESULT(m_threads[i]->Wait());
		--m_workingThreadsNum;
	}
	m_bRun = false;
	return OK;
}

int WinThreadPool::GetWorkThreadsNum() {
	return m_workingThreadsNum;
}

int WinThreadPool::GetThreadsNum() {
	return m_threadsNum;
}

int WinThreadPool::GetTaskCount() {
	int size;
	EnterCriticalSection(&m_cs);
	size = m_tasks.size();
	LeaveCriticalSection(&m_cs);
	return size;
}

Status WinThreadPool::SetThreadsNum(int n) {
	CHECK_ERROR(!m_bRun && !m_workingThreadsNum);
	CHECK_ERROR(n > 0);
	CHECK_ERROR(n <= m_maxThreadsNum);

	if (n != m_threadsNum) {
		if (n > m_threadsNum) {
			while (m_threadsNum < n) {
				m_threads.push_back(GGetWinThread(this));
				++m_threadsNum;
			}
		}
		else {
			while (m_threadsNum > n) {
				--m_threadsNum;
				Thread* thread = m_threads.back();
				m_threads.pop_back();
				delete thread;
			}
		}
	}
	return OK;
}

int WinThreadPool::GetMaxThreadsNum() {
	return m_maxThreadsNum;
}

Status WinThreadPool::SetMaxThreadsNum(int n) {
	CHECK_ERROR(!m_bRun && !m_workingThreadsNum);
	CHECK_ERROR(n > 0);

	if (n != m_maxThreadsNum) {
		while (m_threadsNum > n) {
			--m_threadsNum;
			Thread* thread = m_threads.back();
			m_threads.pop_back();
			delete thread;
		}
		m_maxThreadsNum = n;

		CloseHandle(m_hSemaphore);
		m_hSemaphore = CreateSemaphore(NULL, 0, m_maxThreadsNum, NULL);
	}
	return OK;
}

Status WinThreadPool::AddTask(Task* task) {
	if (task) {
		EnterCriticalSection(&m_cs);
		m_tasks.push(task);
		LeaveCriticalSection(&m_cs);
	}
	return OK;
}

Task* WinThreadPool::GetTask() {
	Task *task;
	EnterCriticalSection(&m_cs);
	if (m_tasks.empty())
		task = NULL;
	else {
		task = m_tasks.front();
		m_tasks.pop();
	}
	LeaveCriticalSection(&m_cs);
	return task;
}

bool WinThreadPool::IsRunning() {
	return m_bRun;
}

Status WinThreadPool::Request() {
	CHECK_ERROR(m_hSemaphore);

	DWORD r = ::WaitForSingleObject(m_hSemaphore, INFINITE);
	CHECK_ERROR(r == WAIT_OBJECT_0);

	return OK;
}

Status WinThreadPool::Release() {
	EnterCriticalSection(&m_cs);
	CHECK_ERROR(m_hSemaphore);

	DWORD r = ::ReleaseSemaphore(m_hSemaphore, 1, NULL);
	CHECK_ERROR(r);
	LeaveCriticalSection(&m_cs);
	return OK;
}
