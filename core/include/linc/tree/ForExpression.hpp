#pragma once
#include <linc/tree/LoopLabel.hpp>
#include <linc/tree/Expression.hpp>
#include <linc/tree/Declaration.hpp>
#include <linc/tree/Statement.hpp>
#include <linc/tree/VariableDeclaration.hpp>
#include <linc/system/Types.hpp>

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

        ForExpression(std::optional<LoopLabel> label, const Token& for_keyword, const Token& left_parenthesis, const Token& right_parenthesis,
            std::unique_ptr<const VariableDeclaration> declaration, std::unique_ptr<const Expression> expression,
            std::unique_ptr<const Statement> statement, std::unique_ptr<const Expression> body)
            :Expression(for_keyword.info), m_label(std::move(label)), m_forKeyword(for_keyword), m_leftParenthesis(left_parenthesis),
            m_rightParenthesis(right_parenthesis), m_specifier(VariableForSpecifier{std::move(declaration), std::move(expression), std::move(statement)}),
            m_body(std::move(body))
        {
            if(label)
            {
                addToken(label->specifier);
                addTokens(label->identifier->getTokens());
            }
            
            addTokens(std::vector{m_forKeyword, m_leftParenthesis});
            addTokens(std::get<0ul>(m_specifier).variableDeclaration->getTokens());
            addTokens(std::get<0ul>(m_specifier).expression->getTokens());
            addTokens(std::get<0ul>(m_specifier).statement->getTokens());
            addToken(m_rightParenthesis);
            addTokens(m_body->getTokens());
        }

        ForExpression(std::optional<LoopLabel> label, const Token& for_keyword, const Token& left_parenthesis, const Token& right_parenthesis,
            const Token& in_keyword, std::unique_ptr<const IdentifierExpression> value_identifier, std::unique_ptr<const IdentifierExpression> array_identifier,
            std::unique_ptr<const Expression> body)
            :Expression(for_keyword.info), m_label(std::move(label)), m_forKeyword(for_keyword), m_leftParenthesis(left_parenthesis),
            m_rightParenthesis(right_parenthesis), m_specifier(RangeForSpecifier{in_keyword, std::move(value_identifier), std::move(array_identifier)}),
            m_body(std::move(body))
        {
            addTokens(std::vector{m_forKeyword, m_leftParenthesis});
            addTokens(std::get<1ul>(m_specifier).valueIdentifier->getTokens());
            addToken(std::get<1ul>(m_specifier).inKeyword);
            addTokens(std::get<1ul>(m_specifier).arrayIdentifier->getTokens());
            addToken(m_rightParenthesis);
            addTokens(m_body->getTokens());
        }

        inline const std::optional<LoopLabel>& getLabel() const { return m_label; }
        inline const Token& getForKeyword() const { return m_forKeyword; }
        inline const Token& getLeftParenthesis() const { return m_leftParenthesis; }
        inline const Token& getRightParenthesis() const { return m_rightParenthesis; }
        inline const Expression* const getBody() const { return m_body.get(); }
        
        inline const Specifier& getSpecifier() const { return m_specifier; }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            auto label = m_label? std::make_optional(m_label->copy()): std::nullopt;

            if(auto variable_for_specifier = std::get_if<const VariableForSpecifier>(&m_specifier))
            {
                auto variable_declaration = Types::uniqueCast<const VariableDeclaration>(variable_for_specifier->variableDeclaration->clone());    
                return std::make_unique<const ForExpression>(std::move(label), m_forKeyword, m_leftParenthesis, m_rightParenthesis,
                    std::move(variable_declaration), variable_for_specifier->expression->clone(),
                    variable_for_specifier->statement->clone(), m_body->clone());
            }
            else if(auto range_for_specifier = std::get_if<const RangeForSpecifier>(&m_specifier))
            {
                auto value_identifier = Types::uniqueCast<const IdentifierExpression>(range_for_specifier->valueIdentifier->clone());
                auto array_identifier = Types::uniqueCast<const IdentifierExpression>(range_for_specifier->arrayIdentifier->clone());

                return std::make_unique<const ForExpression>(std::move(label), m_forKeyword, m_leftParenthesis, m_rightParenthesis,
                    range_for_specifier->inKeyword, std::move(value_identifier), std::move(array_identifier), m_body->clone());
            }
            else throw LINC_EXCEPTION_OUT_OF_BOUNDS(m_specifier);
        }
    private:
        const std::optional<LoopLabel> m_label;
        const Token m_forKeyword, m_leftParenthesis, m_rightParenthesis;
        const Specifier m_specifier;
        const std::unique_ptr<const Expression> m_body;
    };
}