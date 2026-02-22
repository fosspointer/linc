#pragma once
#include <cstddef>
#include <forward_list>
#include <cassert>
#include <cstdlib>

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
                return Arena::allocate<T>(n);
            }

            void deallocate(pointer p, std::size_t n)
            {
                return Arena::deallocate<T>(p, n);
            }
        };

        template <typename T>
        inline static T* allocate(std::size_t count = 1ul) { return get().allocateImpl<T>(count); }

        template <typename T>
        inline static void deallocate(T* pointer, std::size_t count = 1ul) { get().deallocateImpl<T>(pointer, count); }

        inline static std::size_t getBlockIndex() { return get().m_blockIndex; }
        inline static std::size_t getBlockDistance() { return std::distance(get().m_blocks.begin(), get().m_blocks.end()); }

        inline static void clear() { get().clearImpl(); }
    private:
        Arena() { m_blocks.push_front(Block{}); }
        static Arena& get()
        {
            static Arena arena_allocator;
            return arena_allocator;
        }

        template <typename T>
        T* allocateImpl(std::size_t count)
        {
            static_assert(sizeof(T) <= sizeof(Block), "Cannot allocate specified type T of size greater than the arena block size.");
            auto total_bytes = count * sizeof(T);

            // Maybe would want to have this?
            // Currently Allocator class checks and conditionally throws exception.
            // assert(total_bytes <= sizeof(Block));

            if(m_blockIndex + total_bytes > blockSize) 
            {
                m_blockIndex = 0ul;
                m_blocks.push_front(Block{});
                return static_cast<T*>(static_cast<void*>(m_blocks.front().data));
            }

            else if(total_bytes > blockSize)
            {
                auto allocation = std::malloc(total_bytes);
                m_largeAllocations.push_front(allocation);
                return static_cast<T*>(allocation);
            }

            T* result = static_cast<T*>(static_cast<void*>(m_blocks.front().data + m_blockIndex));
            m_blockIndex += total_bytes;
            return result;
        }

        template <typename T>
        void deallocateImpl(T* pointer, std::size_t count)
        {
            auto deletion_size = count * sizeof(T);
            auto deletion_start_index = m_blockIndex - deletion_size;
            auto actual_start_index = reinterpret_cast<std::size_t>(pointer);

            if(actual_start_index == deletion_start_index)
                m_blockIndex -= deletion_size;
        }

        void clearImpl()
        {
            m_blocks.clear();
            m_blocks.push_front(Block{});
            m_blockIndex = 0ul;

            for(const auto& block: m_largeAllocations)
                std::free(block);

            m_largeAllocations.clear();
        }

        struct Block final { std::byte data[blockSize]; };
        std::forward_list<Block> m_blocks;
        std::forward_list<void*> m_largeAllocations;
        std::size_t m_blockIndex{0ul};
    };

    template<typename T, typename U>
    constexpr bool operator==(const Arena::Allocator<T>&, const Arena::Allocator<U>&) { return true; }

    template<typename T, typename U>
    constexpr bool operator!=(const Arena::Allocator<T>&, const Arena::Allocator<U>&) { return false; }
}
