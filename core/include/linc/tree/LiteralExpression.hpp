#pragma once
#include <linc/system/Reporting.hpp>
#include <linc/tree/Expression.hpp>

namespace linc
{
    class LiteralExpression final : public Expression
    {
    public:
        LiteralExpression(const Token& token)
            :Expression({.tokenList = {token}, .info = token.info}), m_token(token)
        {
            if(!m_token.isLiteral())
                Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Warning, .stage = Reporting::Stage::AST,
                        .message = linc::Logger::format("Literal Expression expected literal token. Got '$' instead.",
                            Token::typeToString(m_token.type))});
            else if(!m_token.value.has_value())
                Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Warning, .stage = Reporting::Stage::AST,
                        .message = "Literal token has no value."});
        }

        inline std::string getValue() const { return m_token.value.value_or(""); }
        inline std::optional<Token::NumberBase> getNumberBase() const { return m_token.numberBase; }
        inline Token::Type getType() const { return m_token.type; }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            return std::make_unique<const LiteralExpression>(m_token); 
        }
    private:
        const Token m_token;
    };
}