#ifndef THREAD_POOL_FACTORY_H
#define THREAD_POOL_FACTORY_H

#include "ThreadPool.h"

class ThreadPoolFactory {
public:
	static ThreadPoolFactory* GetInstance();

	ThreadPool* GetThreadPool();

private:
	static ThreadPoolFactory* factory;
};

#endif