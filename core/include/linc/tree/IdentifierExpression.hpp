#pragma once
#include <linc/system/Reporting.hpp>
#include <linc/tree/Expression.hpp>

namespace linc
{
    class IdentifierExpression final : public Expression 
    {
    public:
        IdentifierExpression(const Token& token)
            :Expression(NodeInfo{.tokenList = {token}, .isValid = token.isValid(), .lineNumber = token.lineNumber}),
            m_identifierToken(token)
        {
            if(m_identifierToken.type != Token::Type::Identifier)
            {
                Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Warning, .stage = Reporting::Stage::AST,
                        .message = linc::Logger::format("Identifier Expression expected identifier token. Got '$' instead.",
                            Token::typeToString(m_identifierToken.type))});
                setValid(false);
            }
            else if(!m_identifierToken.value.has_value())
            {
                Reporting::push(Reporting::Report{
                    .type = Reporting::Type::Warning, .stage = Reporting::Stage::AST,
                    .message = "Identifier token has no value."
                });
            }
        }

        virtual std::vector<const Node*> getChildren() const final override
        {
            return {};
        }

        virtual std::unique_ptr<const Expression> clone_const() const final override
        {
            return std::make_unique<const IdentifierExpression>(m_identifierToken); 
        }

        const Token& getIdentifierToken() const { return m_identifierToken; }
        std::string getValue() const { return m_identifierToken.value.value_or(""); }
    private:
        Token m_identifierToken;
    };
}