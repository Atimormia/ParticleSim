#pragma once
#include <cstddef>
#include <cassert>

namespace core
{

    static constexpr size_t INVALID_INDEX = static_cast<size_t>(-1);

    template <typename T>
    class FreeListPool
    {
    public:
        explicit FreeListPool(size_t capacity)
            : capacity_(capacity), nodes_(new Node[capacity])
        {
            for (size_t i = 0; i < capacity_ - 1; ++i)
            {
                nodes_[i].nextFree = i + 1;
            }
            nodes_[capacity_ - 1].nextFree = INVALID_INDEX;
            freeHead_ = 0;
        }

        ~FreeListPool()
        {
            delete[] nodes_;
        }

        size_t allocate()
        {
            if (freeHead_ == INVALID_INDEX)
                return INVALID_INDEX;

            size_t index = freeHead_;
            freeHead_ = nodes_[index].nextFree;
            nodes_[index].inUse = true;
            return index;
        }

        void deallocate(size_t index)
        {
            assert(index < capacity_);
            assert(nodes_[index].inUse && "Double free");

            nodes_[index].inUse = false;
            nodes_[index].nextFree = freeHead_;
            freeHead_ = index;
        }

        T &get(size_t index) const
        {
            assert(index < capacity_);
            assert(nodes_[index].inUse && "Accessing free node");
            return nodes_[index].value;
        }

        size_t capacity() const { return capacity_; }

    private:
        struct Node
        {
            T value{};
            size_t nextFree = INVALID_INDEX;
            bool inUse = false;
        };

        size_t capacity_;
        Node *nodes_;
        size_t freeHead_ = INVALID_INDEX;
    };
} // namespace core
