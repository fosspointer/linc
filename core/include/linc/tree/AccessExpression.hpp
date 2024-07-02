#pragma once
#include <linc/tree/Expression.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class AccessExpression final : public Expression 
    {
    public:
        AccessExpression(const Token& access_token, std::unique_ptr<const Expression> base, std::unique_ptr<const IdentifierExpression> identifier)
            :Expression(access_token.info), m_accessToken(access_token), m_base(std::move(base)), m_identifier(std::move(identifier))
        {
            addTokens(m_base->getTokens());
            addToken(m_accessToken);
            addTokens(m_identifier->getTokens());
        }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            auto identifier = Types::unique_cast<const IdentifierExpression>(m_identifier->clone());
            return std::make_unique<const AccessExpression>(m_accessToken, m_base->clone(), std::move(identifier));
        }

        inline const Token& getAccessToken() const { return m_accessToken; }
        inline const Expression* const getBase() const { return m_base.get(); }
        inline const IdentifierExpression* const getIdentifier() const { return m_identifier.get(); }
    private:
        const Token m_accessToken;
        const std::unique_ptr<const Expression> m_base;
        const std::unique_ptr<const IdentifierExpression> m_identifier;
    };
}