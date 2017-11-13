# ThreadPool

线程池实现

## 介绍

使用 win32 和 pthread 库分别实现了 Window 和 Linux 下简单的线程池。通过临界区、信号量、互斥量、条件变量等同步手段自己实现了线程的阻塞、唤醒与终止。由线程池对象创建并管理线程生命周期，用户只需要自己实现 Task 的派生类将其添加到线程中即可。注意添加的 Task 会在执行完成后被析构，故添加进去的 Task 必须从堆上分配内存。还有只是简单的测试了一下，不知道苛刻的多线程环境下会不会有bug

## 示例

```c++
// get the thread pool
ThreadPool *pool = ThreadPoolFactory::GetInstance()->GetThreadPool();

// set the number of thread in the thread pool
pool->SetThreadsNum(4);

// implement your own tasks and add concrete tasks to the thread pool
// one thing to be noted is that task will be destructed after running
// pool->AddTask(new ConcreteTask());

// start tasks under multi thread
pool->StartTasks();

// you can do something else while the thread pool is running

// wait for the thread pool to complete the tasks
pool->WaitForThreads();
```
