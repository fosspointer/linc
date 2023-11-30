#pragma once
#include <linc/system/Types.hpp>
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundStatement.hpp>

namespace linc
{
    class BoundVariableDeclarationStatement final : public BoundStatement
    {
    public:
        BoundVariableDeclarationStatement(Types::Type type, const std::string& name, std::unique_ptr<const BoundExpression> value_expression)
            :m_type(type), m_name(name), m_valueExpression(std::move(value_expression))
        {}

        inline Types::Type getType() const { return m_type; }
        inline const std::string& getName() const { return m_name; }
        inline const BoundExpression* getValueExpression() const { return m_valueExpression.get(); }
    private:
        const Types::Type m_type;
        const std::string m_name;
        std::unique_ptr<const BoundExpression> m_valueExpression;
    };
}