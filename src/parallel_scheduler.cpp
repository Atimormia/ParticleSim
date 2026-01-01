#include "core/parallel_scheduler.hpp"
namespace core
{

    ThreadPoolScheduler::ThreadPoolScheduler(size_t threadCount)
    {
        if (threadCount == 0)
            threadCount = 1;

        workers.reserve(threadCount);

        for (size_t i = 0; i < threadCount; ++i)
        {
            workers.emplace_back([this] { workerLoop(); });
        }
    }

    void ThreadPoolScheduler::workerLoop()
    {
        while (true)
        {
            std::function<void()> task;

            {
                std::unique_lock lock(queueMutex);
                queueCV.wait(lock, [&] { return shutdown.load() || !taskQueue.empty(); });

                if (shutdown && taskQueue.empty())
                    return;

                task = std::move(taskQueue.front());
                taskQueue.pop();
            }

            task();

            if (activeTasks.fetch_sub(1) == 1)
            {
                std::lock_guard lock(completionMutex);
                completionCV.notify_all();
            }
        }
    }

    void ThreadPoolScheduler::submit(std::function<void()> task)
    {
        activeTasks.fetch_add(1);

        {
            std::lock_guard lock(queueMutex);
            taskQueue.push(std::move(task));
        }

        queueCV.notify_one();
    }

    void ThreadPoolScheduler::wait()
    {
        std::unique_lock lock(completionMutex);
        completionCV.wait(lock, [&]
                          { return activeTasks.load() == 0; });
    }

    void ThreadPoolScheduler::parallelFor(
        size_t begin,
        size_t end,
        size_t chunkSize,
        const std::function<void(size_t)> &fn)
    {
        if (begin >= end)
            return;

        const size_t total = end - begin;
        const size_t chunks = (total + chunkSize - 1) / chunkSize;

        for (size_t c = 0; c < chunks; ++c)
        {
            const size_t chunkBegin = begin + c * chunkSize;
            const size_t chunkEnd = std::min(chunkBegin + chunkSize, end);

            submit([=]
                   {
            for (size_t i = chunkBegin; i < chunkEnd; ++i)
                fn(i); });
        }

        wait();
    }

    ThreadPoolScheduler::~ThreadPoolScheduler()
    {
        shutdown.store(true);

        {
            std::lock_guard lock(queueMutex);
        }

        queueCV.notify_all();

        for (auto &t : workers)
            t.join();
    }

}