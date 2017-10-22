#ifndef WIN_THREAD_H
#define WIN_THREAD_H

#include "../../include/Thread.h"
#include <Windows.h>

class WinThread : public Thread
{
public:
	explicit WinThread(ThreadPool*);
	~WinThread();

	ThreadPool *GetThreadPool();

	Status Create();
	Status Destroy();

	Status Start();
	Status Wait();
	Status Terminate();

	bool IsRunning();

private:
	static DWORD WINAPI _ThreadProc(LPVOID lpParameter);

	ThreadPool* m_threadPool;
	HANDLE m_hThread;
	bool m_bRun;
};

Thread* GGetWinThread(ThreadPool* pool);

#endif // !WIN_THREAD_H
