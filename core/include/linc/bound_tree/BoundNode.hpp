#pragma once
#include <linc/system/Logger.hpp>
#include <linc/system/PrimitiveValue.hpp>
#include <linc/system/Reporting.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundNode 
    {
    public:
        BoundNode(const Types::type& type)
            :m_type(type)
        {}
        virtual ~BoundNode() = default;

        [[nodiscard]] inline std::string toString() const { return Logger::format("$ (:$)", toStringInner(), Types::toString(m_type)); }
        [[nodiscard]] inline Types::type getType() const { return m_type; }
    protected:
        virtual std::string toStringInner() const = 0;
    private:
        const Types::type m_type;
    };
}