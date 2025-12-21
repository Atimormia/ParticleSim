#pragma once
#include <cstddef>
#include <array>
#include <stdexcept>
#include <assert.h>
#include <memory>
namespace core
{
    template <typename T>
    struct AllocatorStorage
    {
        static_assert(std::is_trivially_destructible_v<T>);

        static constexpr size_t INVALID_INDEX = static_cast<size_t>(-1);

        struct MemoryBlock
        {
            T data{};
            size_t next = INVALID_INDEX;
            bool inUse = false;
        };

        explicit AllocatorStorage(size_t elementCount)
            : elementCount(elementCount),
              memoryBlocks(new MemoryBlock[elementCount])
        {
            if (elementCount == 0)
                throw std::invalid_argument("AllocatorStorage requires positive element count");

            for (size_t i = 0; i < elementCount - 1; ++i)
                memoryBlocks[i].next = i + 1;

            memoryBlocks[elementCount - 1].next = INVALID_INDEX;
            head = 0;
        }

        ~AllocatorStorage()
        {
            delete[] memoryBlocks;
        }

        AllocatorStorage(const AllocatorStorage &) = delete;
        AllocatorStorage &operator=(const AllocatorStorage &) = delete;

        size_t elementCount;
        MemoryBlock *memoryBlocks;
        size_t head;
    };

    template <typename T>
    class PoolAllocator
    {
        static_assert(std::is_trivially_destructible_v<T>);

    public:
        using value_type = T;

        PoolAllocator() noexcept : PoolAllocator(1024) {}

        explicit PoolAllocator(size_t elementCount)
            : storage_(std::make_shared<AllocatorStorage<T>>(elementCount)) {}

        template <typename U, typename = std::enable_if_t<std::is_same_v<U, T>>>
        PoolAllocator(const PoolAllocator<U> &other) noexcept
        {
            static_assert(std::is_convertible_v<U *, T *>,
                          "Allocator copy constructor only allowed for compatible types");
            storage_ = other.storage_;
        }

        T *allocate(size_t n = 1)
        {
            if (n != 1)
                throw std::bad_alloc();

            if (storage_->head == AllocatorStorage<T>::INVALID_INDEX)
                throw std::bad_alloc();

            auto &block = storage_->memoryBlocks[storage_->head];
            assert(!block.inUse);

            block.inUse = true;
            storage_->head = block.next;
            return &block.data;
        }

        void deallocate(T *ptr, size_t n = 1)
        {
            if (n != 1)
                throw std::invalid_argument("PoolAllocator can only deallocate single objects");
            if (ptr == nullptr)
                throw std::invalid_argument("Cannot deallocate nullptr");

            using Block = typename AllocatorStorage<T>::MemoryBlock;
            Block *blockPtr = reinterpret_cast<Block *>(
                reinterpret_cast<char *>(ptr) - offsetof(Block, data));
            if (blockPtr < storage_->memoryBlocks || blockPtr >= storage_->memoryBlocks + storage_->elementCount)
            {
                throw std::invalid_argument("Pointer not from this allocator");
            }

            assert(blockPtr->inUse && "Double free detected");

            blockPtr->next = storage_->head;
            storage_->head = blockPtr - storage_->memoryBlocks;
            blockPtr->inUse = false;
        }

        size_t capacity() const
        {
            return storage_->elementCount_;
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

    private:
        template <typename>
        friend class PoolAllocator;

        std::shared_ptr<AllocatorStorage<T>> storage_;
    };

}