#pragma once
#include <linc/system/Types.hpp>
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/bound_tree/BoundStatement.hpp>

namespace linc
{
    class BoundVariableDeclarationStatement final : public BoundStatement
    {
    public:
        BoundVariableDeclarationStatement(Types::Type type, const std::string& name, bool variable, std::unique_ptr<const BoundExpression> value_expression)
            :BoundStatement(Types::Type::_void), m_type(type), m_name(name), m_valueExpression(std::move(value_expression)), m_variable(variable)
        {}

        inline Types::Type getType() const { return m_type; }
        inline const std::string& getName() const { return m_name; }
        inline const BoundExpression* const getValueExpression() const { return m_valueExpression.get(); }
        inline bool getVariable() const { return m_variable; }
    private:
        const Types::Type m_type;
        const std::string m_name;
        const bool m_variable;
        const std::unique_ptr<const BoundExpression> m_valueExpression;
    };
}