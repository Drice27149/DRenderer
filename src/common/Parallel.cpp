#include "Parallel.hpp"


void workerThreadFunc()
{
    std::unique_lock<std::mutex> lock(m);
    while(!shutdownThreads){
        if(!workList) cv.wait(lock);
        else {
            auto& loop = *workList;
            auto begin = loop.nextIndex;
            auto end = loop.nextIndex + loop.chunkSize > loop.count ? loop.count : loop.nextIndex + loop.chunkSize;
            loop.nextIndex = end;
            if(end == loop.count) workList = nullptr;
            ++loop.activeThreads;
            lock.unlock();
            for(auto i = begin; i < end; i++) loop.func(i);
            lock.lock();
            --loop.activeThreads;
        }
    }
}

void parallelInit()
{
    auto maxThreads = std::thread::hardware_concurrency();
    // hack
    if(maxThreads > 5) maxThreads = 5;
    for(auto i = 0; i < maxThreads-1; i++)
        threads.emplace_back(workerThreadFunc);
}

void parallelCleanUp()
{
    {
        std::lock_guard<std::mutex> guard(m);
        shutdownThreads = true;
    }

    cv.notify_all();

    for(auto& thread: threads) thread.join();
    threads.clear();
    shutdownThreads = false;
}

void parallelFor1D(std::function<void(unsigned long long)>&& func, unsigned long long count, unsigned long long chunkSize)
{
    if(count <= chunkSize){
        for(auto i = 0; i < count; i++) func(i);
        return ;
    }

    ParallelForLoop loop(std::move(func), count, chunkSize);
    std::unique_lock<std::mutex> lock(m);
    workList = &loop;
    cv.notify_all();

    while(!loop.isFinish())
    {
        auto begin = loop.nextIndex;
        auto end = loop.nextIndex + loop.chunkSize > loop.count ? loop.count : loop.nextIndex + loop.chunkSize;
        loop.nextIndex = end;
        if(end == loop.count) workList = nullptr;
        ++loop.activeThreads;
        lock.unlock();
        for(auto i = begin; i < end; i++) loop.func(i);
        lock.lock();
        --loop.activeThreads;
    }
}