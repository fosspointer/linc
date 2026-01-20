#pragma once
#include <string>
#include <vector>
#include <stack>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <linc/system/Arena.hpp>

namespace linc
{
    // Mapping STL containers that use allocators
    template<typename CHAR_TYPE, typename TRAITS = std::char_traits<CHAR_TYPE>>
    using BasicString = std::basic_string<CHAR_TYPE, TRAITS, typename Arena::Allocator<CHAR_TYPE>>;
    using String = BasicString<char>;

    template<typename T>
    using Vector = std::vector<T, typename Arena::Allocator<T>>;

    template<typename T>
    using Stack = std::stack<T, typename Arena::Allocator<T>>;

    template<typename T>
    using Deque = std::deque<T, typename Arena::Allocator<T>>;

    template<typename T>
    using UnorderedMap = std::unordered_map<T, typename Arena::Allocator<T>>;

    template<typename T>
    using UnorderedSet = std::unordered_set<T, typename Arena::Allocator<T>>;
}
