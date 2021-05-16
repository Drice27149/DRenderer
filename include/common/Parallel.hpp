#include <vector>
#include <thread>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <iostream>

class ParallelForLoop {
public:
    ParallelForLoop(std::function<void(unsigned long long)>&& func, unsigned long long count, unsigned long long chunkSize):
    func(std::move(func)), count(count), chunkSize(chunkSize)
    {}

public:
    bool isFinish() const {
        return nextIndex == count && activeThreads == 0;
    }

public:
    std::function<void(int64_t)> func;
    // 任务总数
    unsigned long long count;
    // 下一个任务的 ID
    unsigned long long nextIndex = 0;
    // 一个线程分配的任务量
    unsigned long long chunkSize;
    // 目前可用线程
    int activeThreads = 0;
    // ParallelForLoop* next; 迷, 干啥用的
};

static std::vector<std::thread> threads;
static auto shutdownThreads = false;
static ParallelForLoop* workList = nullptr;
static std::mutex m;
static std::condition_variable cv;

void workerThreadFunc();

void parallelFor1D(std::function<void(unsigned long long)>&& func, unsigned long long count, unsigned long long chunkSize);

void parallelInit();

void parallelCleanUp();