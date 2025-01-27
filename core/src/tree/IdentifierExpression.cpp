#include <linc/tree/IdentifierExpression.hpp>
#include <linc/tree/GenericClause.hpp>

namespace linc
{
    IdentifierExpression::IdentifierExpression(const Token& token, std::unique_ptr<const GenericClause> generic)
        :Expression(NodeInfo{.tokenList = {token}, .info = token.info}), m_identifierToken(token), m_generic(std::move(generic))
    {
        LINC_NODE_ASSERT_IDENTIFIER(m_identifierToken);
        addToken(m_identifierToken);
        if(m_generic)
            addTokens(m_generic->getTokens());
    }

    IdentifierExpression::~IdentifierExpression() = default;

    std::unique_ptr<const Expression> IdentifierExpression::clone() const
    {
        return std::make_unique<const IdentifierExpression>(m_identifierToken, m_generic? m_generic->clone(): nullptr);
    }
}