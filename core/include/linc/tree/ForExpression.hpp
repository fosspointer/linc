#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/tree/Declaration.hpp>
#include <linc/tree/Statement.hpp>
#include <linc/tree/VariableDeclaration.hpp>

namespace linc
{
    class ForExpression final : public Expression
    {
    public:
        struct VariableForSpecifier final
        {
            std::unique_ptr<const VariableDeclaration> variableDeclaration;
            std::unique_ptr<const Expression> expression;
            std::unique_ptr<const Statement> statement;
        };

        struct RangeForSpecifier final 
        {
            Token inKeyword;
            std::unique_ptr<const IdentifierExpression> valueIdentifier, arrayIdentifier;
        };

        using Specifier = std::variant<const VariableForSpecifier, const RangeForSpecifier>;

        ForExpression(const Token& for_keyword, const Token& left_parenthesis, const Token& right_parenthesis,
            std::unique_ptr<const VariableDeclaration> declaration, std::unique_ptr<const Expression> expression,
            std::unique_ptr<const Statement> statement, std::unique_ptr<const Statement> body)
            :Expression(for_keyword.info), m_forKeyword(for_keyword), m_leftParenthesis(left_parenthesis), m_rightParenthesis(right_parenthesis),
            m_specifier(VariableForSpecifier{std::move(declaration), std::move(expression), std::move(statement)}), m_body(std::move(body))
        {}

        ForExpression(const Token& for_keyword, const Token& left_parenthesis, const Token& right_parenthesis, const Token& in_keyword,
            std::unique_ptr<const IdentifierExpression> value_identifier, std::unique_ptr<const IdentifierExpression> array_identifier,
            std::unique_ptr<const Statement> body)
            :m_forKeyword(for_keyword), m_leftParenthesis(left_parenthesis), m_rightParenthesis(right_parenthesis),
            m_specifier(RangeForSpecifier{in_keyword, std::move(value_identifier), std::move(array_identifier)}), m_body(std::move(body))
        {}

        inline const Token& getForKeyword() const { return m_forKeyword; }
        inline const Token& getLeftParenthesis() const { return m_leftParenthesis; }
        inline const Token& getRightParenthesis() const { return m_rightParenthesis; }
        inline const Statement* const getBody() const { return m_body.get(); }
        
        inline const Specifier& getSpecifier() const { return m_specifier; }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            if(auto variable_for_specifier = std::get_if<const VariableForSpecifier>(&m_specifier))
            {
                auto variable_declaration = Types::unique_cast<const VariableDeclaration>(variable_for_specifier->variableDeclaration->clone());    

                return std::make_unique<const ForExpression>(m_forKeyword, m_leftParenthesis, m_rightParenthesis,
                    std::move(variable_declaration), variable_for_specifier->expression->clone(),
                    variable_for_specifier->statement->clone(), m_body->clone());
            }
            else if(auto range_for_specifier = std::get_if<const RangeForSpecifier>(&m_specifier))
            {
                auto value_identifier = Types::unique_cast<const IdentifierExpression>(range_for_specifier->valueIdentifier->clone());
                auto array_identifier = Types::unique_cast<const IdentifierExpression>(range_for_specifier->arrayIdentifier->clone());

                return std::make_unique<const ForExpression>(m_forKeyword, m_leftParenthesis, m_rightParenthesis,
                    range_for_specifier->inKeyword, std::move(value_identifier), std::move(array_identifier), m_body->clone());
            }
            else throw LINC_EXCEPTION_OUT_OF_BOUNDS((std::variant<const VariableForSpecifier, const RangeForSpecifier>));
        }
    private:
        const Token m_forKeyword, m_leftParenthesis, m_rightParenthesis;
        const Specifier m_specifier;
        const std::unique_ptr<const Statement> m_body;
    };
}