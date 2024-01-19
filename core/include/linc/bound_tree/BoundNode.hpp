#pragma once
#include <linc/system/Logger.hpp>
#include <linc/system/TypedValue.hpp>
#include <linc/system/Reporting.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundNode 
    {
    public:
        BoundNode(Types::Type type)
            :m_type(type)
        {}
        virtual ~BoundNode() = default;

        [[nodiscard]] inline std::string toString() const { return Logger::format("$ (:$)", toStringInner(), Types::toString(m_type)); }
        [[nodiscard]] inline Types::Type getType() const { return m_type; }
    protected:
        virtual std::string toStringInner() const = 0;
    private:
        const Types::Type m_type;
    };
}