#pragma once
#include <cstddef>
#include <forward_list>
#include <cassert>
#include <cstdlib>
#include <memory>

namespace linc
{
    class Arena final
    {
    public:
        constexpr static const std::size_t blockSize = 65536ul;

        template <typename T>
        class Allocator
        {
        public:
            using value_type = T;
            using pointer = T*;
            using const_pointer = const T*;
            using reference = T&;
            using const_reference = const T&;
            using size_type = std::size_t;
            using difference_type = std::ptrdiff_t;

            Allocator() = default;

            constexpr size_type max_size() { return Arena::blockSize / sizeof(T); }
            
            [[nodiscard]] inline pointer allocate(size_type n)
            {
                return Arena::allocate<T>(n, alignof(T));
            }

            void deallocate([[maybe_unused]] pointer p, [[maybe_unused]] size_type n) {}
        };

        template <typename T>
        inline static T* allocate(std::size_t count = 1ul, std::size_t alignment = alignof(T))
        {
            return get().allocateImpl<T>(count, alignment);
        }

        inline static std::size_t getBlockDistance() { return std::distance(get().m_blocks.begin(), get().m_blocks.end()); }

        inline static void clear() { get().clearImpl(); }
    private:
        Arena()
        {
            m_blocks.push_front(Block{});
            m_pointer = static_cast<void*>(m_blocks.front().data);
        }

        static Arena& get()
        {
            static Arena arena_allocator;
            return arena_allocator;
        }

        template <typename T>
        T* allocateImpl(std::size_t count, std::size_t alignment)
        {
            auto aligned_bytes = count * sizeof(T) + alignment;
            if(aligned_bytes > blockSize)
            {
                auto allocation = std::malloc(aligned_bytes);
                m_largeAllocations.push_front(allocation);
                return static_cast<T*>(allocation);
            }

            else if(aligned_bytes > m_bytesLeft) 
            {
                m_blocks.push_front(Block{});
                m_bytesLeft = blockSize;
                m_pointer = static_cast<void*>(m_blocks.front().data);
                return performAlignedAllocation<T>(count, alignment);
            }

            return performAlignedAllocation<T>(count, alignment);
        }

        template <typename T>
        T* performAlignedAllocation(std::size_t count, std::size_t alignment)
        {
            const std::size_t total_size{count * sizeof(T)};
            if(!std::align(alignment, total_size, m_pointer, m_bytesLeft))
                return nullptr;

            T* result = std::launder(reinterpret_cast<T*>(m_pointer));
            m_pointer = static_cast<void*>(static_cast<std::byte*>(m_pointer) + total_size);
            m_bytesLeft -= total_size;
            return result;
        }

        void clearImpl()
        {
            m_blocks.clear();
            m_blocks.push_front(Block{});
            m_pointer = static_cast<void*>(m_blocks.front().data);

            for(const auto& block: m_largeAllocations)
                std::free(block);

            m_largeAllocations.clear();
        }

        struct Block final { std::byte data[blockSize]; };
        std::forward_list<Block> m_blocks;
        std::forward_list<void*> m_largeAllocations;
        void* m_pointer{nullptr};
        std::size_t m_bytesLeft{blockSize};
    };

    template<typename T, typename U>
    constexpr bool operator==(const Arena::Allocator<T>&, const Arena::Allocator<U>&) { return true; }

    template<typename T, typename U>
    constexpr bool operator!=(const Arena::Allocator<T>&, const Arena::Allocator<U>&) { return false; }
}
