#pragma once
#include <linc/bound_tree/BoundNode.hpp>

namespace linc
{
    class BoundExpression : public BoundNode
    {
    public:
        BoundExpression(const Types::type& type)
            :m_type(type)
        {}
        
        [[nodiscard]] inline const Types::type& getType() const { return m_type; };
        virtual ~BoundExpression() = default;
        virtual std::unique_ptr<const BoundExpression> clone() const = 0;

        [[nodiscard]] virtual std::string toString() const final override
        {
            std::string result{Colors::push(Colors::Color::Yellow)};
            result.append(toStringInner());
            result.append(Colors::pop());
            Logger::append(result, " (:$)", PrimitiveValue(m_type));
            return result;
        }
    private:
        const Types::type m_type;
    };
}