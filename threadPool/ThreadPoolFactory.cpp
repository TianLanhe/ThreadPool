#include "../include/ThreadPoolFactory.h"

#if defined(WIN32)
    #include "window/WinThreadPool.h"
#elif defined(linux) || defined(__APPLE__)
    #include "linux/LinuxThreadPool.h"
#endif

ThreadPoolFactory* ThreadPoolFactory::factory = nullptr;

ThreadPoolFactory* ThreadPoolFactory::GetInstance() {
	if (factory == nullptr)
		factory = new ThreadPoolFactory();
	return factory;
}

ThreadPool* ThreadPoolFactory::GetThreadPool() {
#if defined(WIN32)
	return new WinThreadPool();
#elif defined(linux) || defined(__APPLE__)
    return new LinuxThreadPool();
#endif

    return nullptr;
}
