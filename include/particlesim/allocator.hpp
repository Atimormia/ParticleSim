#pragma once
#include <cstddef>
namespace particlesim
{
    class IAllocator
    {
    public:
        virtual ~IAllocator() = default;
        virtual void *allocate(size_t size, size_t alignment = alignof(std::max_align_t)) = 0;
        virtual void deallocate(void *ptr) = 0;
    };

    class PoolAllocator : public IAllocator
    {
    public:
        PoolAllocator(size_t blocksNum, size_t blockSize) : blocksNum_(blocksNum), blockSize_(blockSize)
        {
            if (blockSize_ < sizeof(void *))
            {
                blockSize_ = sizeof(void *);
            }
            memoryBlock_ = new char[blocksNum_ * blockSize_];

            head = memoryBlock_;
            char *current = head;
            for (size_t i = 0; i < blocksNum_ - 1; ++i)
            {
                char *next = current + blockSize_;
                *reinterpret_cast<void **>(current) = next;
                current = next;
            }
            *reinterpret_cast<void **>(current) = nullptr; // last block points to nullptr
        };
        ~PoolAllocator()
        {
            delete[] memoryBlock_;
        };

        void *allocate(size_t size, size_t alignment = alignof(std::max_align_t)) override
        {
            if (size > blockSize_ || head == nullptr)
            {
                return nullptr;
            }
            void *ptr = head;
            head = *reinterpret_cast<char **>(head); // move head to next free block
            return ptr;
        };
        void deallocate(void *ptr) override
        {
            if (ptr == nullptr)
            {
                return;
            }

            *reinterpret_cast<void **>(ptr) = head;
            head = static_cast<char *>(ptr);
        };

    private:
        char *memoryBlock_ = nullptr;
        size_t blocksNum_ = 0;
        size_t blockSize_ = 0;
        char *head = nullptr;
    };
}