#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/tree/IdentifierExpression.hpp>

namespace linc
{
    class StructureInitializerExpression final : public Expression
    {
    public:
        struct Argument final
        {
            const Token accessSpecifier, equalitySpecifier, separator;
            std::unique_ptr<const IdentifierExpression> identifier;
            std::unique_ptr<const Expression> value;
        };

        StructureInitializerExpression(const Token& left_brace, std::unique_ptr<const IdentifierExpression> identifier,
            std::vector<Argument> arguments)
            :Expression(left_brace.info), m_leftBrace(left_brace), m_identifier(std::move(identifier)),
            m_arguments(std::move(arguments))
        {
            addTokens(m_identifier->getTokens());
            addToken(m_leftBrace);
            
            for(const auto& argument: m_arguments)
            {
                addToken(argument.accessSpecifier);
                addTokens(argument.identifier->getTokens());
                addToken(argument.equalitySpecifier);
                addTokens(argument.value->getTokens());
                addToken(argument.separator);
            }
        }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            std::vector<Argument> arguments;
            arguments.reserve(m_arguments.size());

            for(const auto& argument: m_arguments)
            {
                auto identifier = Types::unique_cast<const IdentifierExpression>(argument.identifier->clone());
                arguments.push_back(Argument{
                    .accessSpecifier = argument.accessSpecifier,
                    .equalitySpecifier = argument.equalitySpecifier,
                    .separator = argument.separator,
                    .identifier = std::move(identifier),
                    .value = argument.value->clone()});
            }

            auto identifier = Types::unique_cast<const IdentifierExpression>(m_identifier->clone());
            return std::make_unique<const StructureInitializerExpression>(m_leftBrace, std::move(identifier), std::move(arguments));
        }

        const Token& getLeftBrace() const { return m_leftBrace; }
        const IdentifierExpression* const getIdentifier() const { return m_identifier.get(); }
        const std::vector<Argument>& getArguments() const { return m_arguments; }
    private:
        const Token m_leftBrace;
        const std::unique_ptr<const IdentifierExpression> m_identifier;
        const std::vector<Argument> m_arguments;
    };
}