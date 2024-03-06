#pragma once
#include <linc/system/Reporting.hpp>
#include <linc/tree/Declaration.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/tree/TypeExpression.hpp>

namespace linc
{
    class VariableDeclaration final : public Declaration
    {
    public:
        class ValueAssignment
        {
        public:
            ValueAssignment(const Token& assignment_operator, std::unique_ptr<const Expression> expression)
                :m_assignmentOperator(assignment_operator), m_expression(std::move(expression))
            {}

            ValueAssignment(ValueAssignment&& other)
                :m_assignmentOperator(other.m_assignmentOperator), m_expression(std::move(other.m_expression))
            {}

            ValueAssignment(const ValueAssignment& other)
                :m_assignmentOperator(other.m_assignmentOperator), m_expression(other.m_expression->cloneConst())
            {}

            ValueAssignment& operator=(const ValueAssignment& other)
            {
                this->m_assignmentOperator = other.m_assignmentOperator;
                this->m_expression = other.m_expression->cloneConst();
                return *this;
            }

            ValueAssignment& operator=(ValueAssignment&& other)
            {
                this->m_assignmentOperator = other.m_assignmentOperator;
                this->m_expression = std::move(other.m_expression);
                return *this;
            }

            inline const Token& getAssignmentOperator() const { return m_assignmentOperator; }
            inline const Expression* const getExpression() const { return m_expression.get(); }
        private:
            Token m_assignmentOperator;
            std::unique_ptr<const Expression> m_expression;
        };

        VariableDeclaration(const Token& type_specifier, std::unique_ptr<const TypeExpression> type,
            std::unique_ptr<const IdentifierExpression> identifier, std::optional<ValueAssignment> default_value)
            :Declaration(NodeInfo{.tokenList = {}, .info = type->getTokenInfo()}), m_typeSpecifier(type_specifier),
            m_type(std::move(type)), m_identifier(std::move(identifier)), m_defaultValue(std::move(default_value))
        {
            addTokens(m_type->getTokens());
            addTokens(m_identifier->getTokens());
            if(m_defaultValue.has_value())
            {
                addToken(m_defaultValue->getAssignmentOperator());
                addTokens(m_defaultValue->getExpression()->getTokens());
            }
        }

        virtual std::vector<const Node*> getChildren() const final override 
        {
            if(m_defaultValue.has_value())
                return {m_type.get(), m_identifier.get(), m_defaultValue->getExpression()};
            else return {m_type.get(), m_identifier.get()};
        }

        virtual std::unique_ptr<const Declaration> cloneConst() const final override
        {
            auto type = Types::unique_cast<const TypeExpression>(m_type->cloneConst());
            auto identifier = Types::unique_cast<const IdentifierExpression>(m_identifier->cloneConst());

            return std::make_unique<const VariableDeclaration>(
                m_typeSpecifier, std::move(type), std::move(identifier),
                m_defaultValue.has_value()? std::make_optional(ValueAssignment(
                    m_defaultValue->getAssignmentOperator(),
                    m_defaultValue->getExpression()->cloneConst())
                ): std::nullopt
            );
        }

        inline const Token& getTypeSpecifier() const { return m_typeSpecifier; }
        inline const TypeExpression* const getType() const { return m_type.get(); }
        inline const IdentifierExpression* const getIdentifier() const { return m_identifier.get(); }
        inline const std::optional<ValueAssignment>& getDefaultValue() const { return m_defaultValue; }
    private:
        const Token& m_typeSpecifier;
        const std::unique_ptr<const TypeExpression> m_type;
        const std::unique_ptr<const IdentifierExpression> m_identifier;
        const std::optional<ValueAssignment> m_defaultValue;
    };
}