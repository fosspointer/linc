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
            LINC_NODE_ASSERT_LITERAL(m_token);
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