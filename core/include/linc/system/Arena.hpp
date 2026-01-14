#pragma once
#include <cstddef>
#include <forward_list>
#include <cassert>
#include <linc/system/Logger.hpp>

namespace linc
{
    class Arena final
    {
    public:
        constexpr static const std::size_t blockSize = 32768;

        template <typename T>
        inline static T* allocate(std::size_t count = 1ul) { return get().allocateImpl<T>(count); }
        inline static void printInfo() { return get().printInfoImpl(); }
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

            assert(total_bytes <= sizeof(Block));

            if(m_blockIndex + total_bytes > blockSize) 
            {
                m_blockIndex = total_bytes;
                m_blocks.push_front(Block{});
                return reinterpret_cast<T*>(m_blocks.front().data);
            }

            T* result = reinterpret_cast<T*>(m_blocks.front().data + m_blockIndex);
            m_blockIndex += total_bytes;
            return result;
        }

        void printInfoImpl()
        {
            Logger::println("Allocated blocks: $, allocation index: $ bytes", std::distance(m_blocks.begin(), m_blocks.end()), m_blockIndex);
        }
        struct Block final { std::byte data[blockSize]; };
        std::forward_list<Block> m_blocks;
        std::size_t m_blockIndex{0ul};
    };
}