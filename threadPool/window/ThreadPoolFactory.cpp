#include "../../include/ThreadPoolFactory.h"
#include "WinThreadPool.h"

ThreadPoolFactory* ThreadPoolFactory::factory = NULL;

ThreadPoolFactory* ThreadPoolFactory::GetInstance() {
	if (factory == NULL)
		factory = new ThreadPoolFactory();
	return factory;
}

ThreadPool* ThreadPoolFactory::GetThreadPool() {
	return new WinThreadPool();
}
