#pragma once
#include <linc/system/Types.hpp>
#include <linc/system/Reporting.hpp>
#include <linc/tree/Expression.hpp>

namespace linc
{
    class LiteralExpression final : public Expression
    {
    public:
        LiteralExpression(const Token& token)
            :Expression({.tokenList = {token}, .isValid = false, .lineNumber = token.lineNumber}), m_type(token.type) 
        {
            if(!token.isLiteral())
            {
                Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Warning, .stage = Reporting::Stage::AST,
                        .message = linc::Logger::format("Literal Expression expected literal token. Got '$' instead.",
                            Token::typeToString(token.type))});
            }
            else if(!token.value.has_value())
            {
                Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Warning, .stage = Reporting::Stage::AST,
                        .message = "Literal token has no value."});
            }
            else
            {
                m_value = token.value.value();
                setValid(true);
            }
        }

        inline const std::string& getValue() const { return m_value; }
        inline Token::Type getType() const { return m_type; }

        virtual std::vector<const Node*> getChildren() const final override 
        {
            return {};
        }
    private:
        Token::Type m_type;
        std::string m_value;
    };
}