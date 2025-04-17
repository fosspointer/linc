#pragma once
#include <memory>
#include <linc/system/Exception.hpp>

namespace linc
{
    class Memory
    {
    public:
        Memory() = delete;
        template <typename To, typename From> 
        [[nodiscard]] static std::unique_ptr<To> uniqueCastDynamic(std::unique_ptr<From> p)
        {
            std::unique_ptr<To> result(dynamic_cast<To*>(p.get()));
            
            if(result)
            {
                p.release();
                return result;
            }
            return nullptr;
        }

        template <typename To, typename From> 
        [[nodiscard]] static std::unique_ptr<To> uniqueCast(std::unique_ptr<From> p)
        {
            std::unique_ptr<To> result(static_cast<To*>(p.get()));
            p.release();
            return result;
        }

        template <typename T>
        [[nodiscard]] static std::vector<std::unique_ptr<const T>> cloneNodes(const std::vector<std::unique_ptr<const T>>* nodes)
        {
            if(!nodes)
                throw LINC_EXCEPTION_ILLEGAL_NULL(nodes);
            
            std::vector<std::unique_ptr<const T>> result;
            
            result.reserve(nodes->size());
            for(const auto& node: *nodes)
                result.push_back(node->clone());

            return result;
        }

        template <typename T, typename U>
        [[nodiscard]] static std::unordered_map<T, std::unique_ptr<const U>> cloneNodeMap(const std::unordered_map<T, std::unique_ptr<const U>>* nodes)
        {
            if(!nodes)
                throw LINC_EXCEPTION_ILLEGAL_NULL(nodes);
            
            std::unordered_map<T, std::unique_ptr<const U>> result;
            
            result.reserve(nodes->size());
            for(const auto& node: *nodes)
                result.insert(std::pair(node.first, node.second->clone()));

            return result;
        }
    };
}