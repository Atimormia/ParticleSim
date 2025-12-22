#pragma once
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <cassert>
#include <new>

namespace core
{
    struct PoolStorage
    {
        struct Block
        {
            Block *next;
#ifndef NDEBUG
            bool inUse = false;
#endif
        };

        PoolStorage(size_t blockSize, size_t capacity)
            : blockSize(blockSize), capacity(capacity)
        {
            if (capacity == 0)
                throw std::invalid_argument("Capacity must be > 0");

            memory = ::operator new(blockSize * capacity);
            freeList = nullptr;

            for (size_t i = 0; i < capacity; ++i)
            {
                auto *block = reinterpret_cast<Block *>(
                    static_cast<char *>(memory) + i * blockSize);
                block->next = freeList;
#ifndef NDEBUG
                block->inUse = false;
#endif
                freeList = block;
            }
        }

        ~PoolStorage()
        {
            ::operator delete(memory);
        }

        void *allocate()
        {
            if (!freeList)
                throw std::bad_alloc();

            Block *block = freeList;
            freeList = block->next;

#ifndef NDEBUG
            assert(!block->inUse && "Allocating already-used block");
            block->inUse = true;
#endif
            return block;
        }

        void deallocate(void *ptr)
        {
            auto *block = static_cast<Block *>(ptr);

#ifndef NDEBUG
            assert(block->inUse && "Double free detected");
            block->inUse = false;
#endif
            block->next = freeList;
            freeList = block;
        }

        bool owns(void *ptr) const
        {
            auto begin = static_cast<char *>(memory);
            auto end = begin + blockSize * capacity;
            return ptr >= begin && ptr < end &&
                   ((static_cast<char *>(ptr) - begin) % blockSize == 0);
        }

        void *memory;
        size_t blockSize;
        size_t capacity;
        Block *freeList;
    };

    template <typename T>
    class PoolAllocator
    {
    public:
        using value_type = T;

        PoolAllocator() : PoolAllocator(1024) {}

        explicit PoolAllocator(size_t capacity)
            : storage_(std::make_shared<PoolStorage>(sizeof(T), capacity)) {}

        template <typename U>
        PoolAllocator(const PoolAllocator<U> &other) noexcept
            : storage_(other.storage_) {}

        T *allocate(size_t n = 1)
        {
            if (n != 1)
                throw std::bad_alloc();

            return static_cast<T *>(storage_->allocate());
        }

        void deallocate(T *ptr, size_t n = 1)
        {
            if (n != 1)
                throw std::bad_alloc();

            storage_->deallocate(ptr);
        }

        template <typename U>
        bool operator==(const PoolAllocator<U> &other) const noexcept
        {
            return storage_.get() == other.storage_.get();
        }

        template <typename U>
        bool operator!=(const PoolAllocator<U> &other) const noexcept
        {
            return !(*this == other);
        }

        size_t capacity() const
        {
            return storage_->capacity;
        }

    private:
        template <typename>
        friend class PoolAllocator;

        std::shared_ptr<PoolStorage> storage_;
    };
}
