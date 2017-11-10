#include "../include/ThreadPoolFactory.h"
#ifdef defined(WIN32)
    #include "window/WinThreadPool.h"
#elif defined(linux)
    #include "linux/LinuxThreadPool.h"
#endif

ThreadPoolFactory* ThreadPoolFactory::factory = NULL;

ThreadPoolFactory* ThreadPoolFactory::GetInstance() {
	if (factory == NULL)
		factory = new ThreadPoolFactory();
	return factory;
}

ThreadPool* ThreadPoolFactory::GetThreadPool() {
#if defined(WIN32)
	return new WinThreadPool();
#elif defined(linux)
    return new LinuxThreadPool();
#else
    return NULL;
#elif
}
