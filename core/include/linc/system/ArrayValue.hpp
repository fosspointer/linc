#pragma once
#include <linc/system/Exception.hpp>
#include <linc/system/Types.hpp>
#include <linc/Include.hpp>
#include <cstring>
#include <cstdlib>

namespace linc
{
    class ArrayValue final
    {
    public:
        ArrayValue(const ArrayValue& other)
            :m_elementSize(other.m_elementSize), m_type(other.m_type)
        {
            m_data.resize(other.m_data.size());
            if(s_stringFunction(*this, [this, &other](std::size_t i){
                new(&reinterpret_cast<Types::string*>(m_data.data())[i])
                    Types::string{reinterpret_cast<const Types::string*>(other.m_data.data())[i]};
            }))
            {
                m_data = other.m_data;
            }
        }

        ArrayValue(ArrayValue&& other)
            :m_elementSize(other.m_elementSize), m_type(other.m_type), m_data(std::move(other.m_data))
        {}

        ArrayValue& operator=(const ArrayValue& other)
        {
            m_elementSize = other.m_elementSize;
            m_type = other.m_type;
            m_data.resize(other.m_data.size());
            if(s_stringFunction(*this, [this, &other](std::size_t i){
                new(&reinterpret_cast<Types::string*>(m_data.data())[i])
                    Types::string{reinterpret_cast<const Types::string*>(other.m_data.data())[i]};
            }))
            {
                m_data = other.m_data;
            }
            return *this;
        }

        ArrayValue& operator=(ArrayValue&& other)
        {
            m_elementSize = other.m_elementSize;
            m_type = other.m_type;
            m_data = std::move(other.m_data);
            return *this;
        }
        
        ArrayValue(std::size_t count, std::size_t element_size, const Types::type& type, bool initialize = true);
        ~ArrayValue();

        bool operator==(const ArrayValue&) const = default;
        bool operator!=(const ArrayValue&) const = default;
        ArrayValue operator+(const ArrayValue& other) const;

        static ArrayValue fromDefault(const Types::type& type, std::size_t count);
        std::size_t getCount() const { return m_data.size() / m_elementSize; }
        void set(std::size_t index, const class Value& value);
        class Value get(std::size_t index) const;
        std::string toString() const;
        std::string toApplicationString() const;
        [[nodiscard]] inline const Types::type& getType() const { return m_type; }
    private:
        static const std::function<bool(ArrayValue&, std::function<void(std::size_t)>)> s_stringFunction;
        static ArrayValue fromDefault(Types::type::Primitive primitive, std::size_t count);
        void set(std::size_t index, const class PrimitiveValue& value);
        class PrimitiveValue getPrimitive(std::size_t index) const;
        std::size_t m_elementSize;
        Types::type m_type;
        std::vector<std::byte> m_data;
    };
}