#pragma once
#include <functional>
#include <thread>
#include <queue>
#include <mutex>
namespace core
{
    struct IScheduler
    {
        virtual ~IScheduler() = default;

        virtual void submit(std::function<void()> task) = 0;

        virtual void parallelFor(
            size_t begin,
            size_t end,
            size_t chunkSize,
            const std::function<void(size_t)> &fn) = 0;

        virtual void wait() = 0;
    };

    class ThreadPoolScheduler final : public IScheduler
    {
    public:
        explicit ThreadPoolScheduler(size_t threadCount = std::thread::hardware_concurrency());
        ~ThreadPoolScheduler();

        void submit(std::function<void()> task) override;
        void parallelFor(size_t begin, size_t end, size_t chunkSize,
                         const std::function<void(size_t)> &fn) override;
        void wait() override;

        ThreadPoolScheduler(const ThreadPoolScheduler&) = delete;
        ThreadPoolScheduler& operator=(const ThreadPoolScheduler&) = delete;
        ThreadPoolScheduler(ThreadPoolScheduler&&) = default;
        ThreadPoolScheduler& operator=(ThreadPoolScheduler&&) = default;

    private:
        void workerLoop();

    private:
        std::vector<std::thread> workers;

        std::queue<std::function<void()>> taskQueue;
        std::mutex queueMutex;
        std::condition_variable queueCV;

        std::condition_variable completionCV;
        std::mutex completionMutex;

        std::atomic<bool> shutdown{false};
        std::atomic<uint32_t> activeTasks{0};
    };

}
