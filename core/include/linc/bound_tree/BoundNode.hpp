#pragma once
#include <linc/system/TypedValue.hpp>

namespace linc
{
    class BoundNode 
    {
    public:
        BoundNode(Types::Type type)
            :m_type(type)
        {}

        virtual ~BoundNode() = default;

        inline std::string toString() const { return Logger::format("$ (:$)", toStringInner(), Types::toString(m_type)); }
        Types::Type getType() const { return m_type; }
    protected:
        virtual std::string toStringInner() const = 0;
    private:
        const Types::Type m_type;
    };
}