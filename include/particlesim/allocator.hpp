#pragma once
#include <cstddef>
#include "particlesim/particle.hpp"
#include <array>
#include <stdexcept>
#include <assert.h>
namespace particlesim
{
    class ParticlePoolAllocator
    {
    public:
        ParticlePoolAllocator(size_t particleCount) : particleCount_(particleCount)
        {
            if (particleCount_ <= 0)
                throw std::invalid_argument("ParticlePoolAllocator requires positive particle count");

            memoryBlocks_ = new MemoryBlock[particleCount_]{};
            head_ = 0;

            for (size_t i = 0; i < particleCount_ - 1; ++i)
            {
                memoryBlocks_[i].next = i + 1;
            }
            memoryBlocks_[particleCount_ - 1].next = -1; // end of free list
        }
        ~ParticlePoolAllocator()
        {
            delete[] memoryBlocks_;
        };

        Particle *allocate()
        {
            if (head_ < 0 || head_ >= particleCount_)
            {
                throw std::bad_alloc();
            }

            Particle *current = &memoryBlocks_[head_].particle;
            memoryBlocks_[head_].inUse = true;
            head_ = memoryBlocks_[head_].next;
            return current;
        }

        void deallocate(Particle *particle)
        {
            if (particle == nullptr)
                throw std::invalid_argument("Cannot deallocate nullptr");

            MemoryBlock *blockPtr = reinterpret_cast<MemoryBlock *>(
                reinterpret_cast<char *>(particle) - offsetof(MemoryBlock, particle));
            if (blockPtr < memoryBlocks_ || blockPtr >= memoryBlocks_ + particleCount_)
            {
                throw std::invalid_argument("Pointer not from this allocator");
            }
            

            MemoryBlock *ptr = reinterpret_cast<MemoryBlock *>(particle);

            assert(ptr->inUse && "Double free detected");

            ptr->next = head_;
            head_ = ptr - memoryBlocks_;
            ptr->inUse = false;
        }

        size_t capacity() const
        {
            return particleCount_;
        }

    private:
        static constexpr size_t INVALID_INDEX = static_cast<size_t>(-1);
        struct MemoryBlock
        {
            Particle particle{};
            size_t next = INVALID_INDEX;
            bool inUse = false;
        };

        const size_t particleCount_ = 0;
        MemoryBlock *memoryBlocks_ = nullptr;

        size_t head_ = INVALID_INDEX;
    };

}