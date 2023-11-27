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
            if(token.type != Token::Type::Identifier)
            {
                Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Warning, .stage = Reporting::Stage::AST,
                        .message = linc::Logger::format("Identifier Expression expected identifier token. Got '$' instead.",
                            Token::typeToString(token.type))});
                setValid(false);
            }
            else if(!token.value.has_value())
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

        const Token& getIdentifierToken() const { return m_identifierToken; }
        std::string getValue() const { return m_identifierToken.value.value_or(""); }
    private:
        Token m_identifierToken;
    };
}