#pragma once
#include <cstddef>
#include <cassert>
#include <type_traits>
#include <stdexcept>

namespace core
{
    class FrameArena
    {
    public:
        FrameArena(size_t size = 1024 * 1024)
        {
            buffer_ = new char[size];
            capacity_ = size;
            head_ = 0;
        }
        
        // deep copy
        FrameArena(const FrameArena &other)
        {
            capacity_ = other.capacity_;
            head_ = other.head_;
            if (other.buffer_)
            {
                buffer_ = new char[capacity_];
                memcpy(buffer_, other.buffer_, head_);
            }
            else
            {
                buffer_ = nullptr;
            }
        }

        // movable
        FrameArena(FrameArena &&other) noexcept
            : buffer_(other.buffer_), capacity_(other.capacity_), head_(other.head_)
        {
            other.buffer_ = nullptr;
            other.capacity_ = 0;
            other.head_ = 0;
        }

        ~FrameArena()
        {
            delete[] buffer_;
        }

        template <typename T>
        T* allocate()
        {
            assert(std::is_trivially_destructible_v<T> && "FrameArena can only allocate trivially destructible types");

            return allocateArray<T>(1);
        }

        template <typename T>
        T* allocateArray(size_t count)
        {
            assert(std::is_trivially_destructible_v<T> && "FrameArena can only allocate trivially destructible types");

            size_t alignment = alignof(T);
            size_t size = sizeof(T) * count;

            size_t current = reinterpret_cast<uintptr_t>(buffer_) + head_;
            size_t aligned = (current + alignment - 1) & ~(alignment - 1);
            size_t padding = aligned - current;

            if (head_ + padding + size > capacity_)
            {
                throw std::bad_alloc();
            }

            head_ += padding;
            T* ptr = reinterpret_cast<T*>(buffer_ + head_);
            head_ += size;
            return ptr;
        }
        
        void reset()
        { 
            // can be subscribed for an event of a global frame manager
            head_ = 0;
        }

        FrameArena &operator=(const FrameArena &other)
        {
            if (this != &other)
            {
                delete[] buffer_;
                capacity_ = other.capacity_;
                head_ = other.head_;
                if (other.buffer_)
                {
                    buffer_ = new char[capacity_];
                    memcpy(buffer_, other.buffer_, head_);
                }
                else
                {
                    buffer_ = nullptr;
                }
            }
            return *this;
        }

        FrameArena &operator=(FrameArena &&other) noexcept
        {
            if (this != &other)
            {
                delete[] buffer_;
                buffer_ = other.buffer_;
                capacity_ = other.capacity_;
                head_ = other.head_;

                other.buffer_ = nullptr;
                other.capacity_ = 0;
                other.head_ = 0;
            }
            return *this;
        }

    private:
        char* buffer_ = nullptr;
        size_t capacity_ = 0;
        size_t head_ = 0;
    };

}