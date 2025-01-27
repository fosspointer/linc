#pragma once
#include <linc/tree/LoopLabel.hpp>
#include <linc/tree/Expression.hpp>
#include <linc/tree/VariantClause.hpp>
#include <linc/tree/LegacyForClause.hpp>
#include <linc/tree/RangedForClause.hpp>
#include <linc/system/Types.hpp>

namespace linc
{
    class ForExpression final : public Expression
    {
    public:
        using ForClause = VariantClause<LegacyForClause, RangedForClause>;
        ForExpression(std::optional<LoopLabel> label, const Token& for_keyword, std::unique_ptr<const ForClause> clause,
            std::unique_ptr<const Expression> body)
            :Expression(for_keyword.info), m_label(std::move(label)), m_forKeyword(for_keyword), m_clause(std::move(clause)), m_body(std::move(body))
        {
            if(m_label)
            {
                addToken(m_label->specifier);
                addTokens(m_label->identifier->getTokens());
            }
            
            addToken(m_forKeyword);
            addTokens(m_clause->getTokens());
            addTokens(m_body->getTokens());
        }

        inline const std::optional<LoopLabel>& getLabel() const { return m_label; }
        inline const Token& getForKeyword() const { return m_forKeyword; }
        inline const ForClause* const getClause() const { return m_clause.get(); }
        inline const Expression* const getBody() const { return m_body.get(); }

        virtual std::unique_ptr<const Expression> clone() const final override
        {
            auto label = m_label? std::make_optional(LoopLabel{.specifier = m_label->specifier,
                .identifier = Types::uniqueCast<const IdentifierExpression>(m_label->identifier->clone())}): std::nullopt;
            return std::make_unique<const ForExpression>(std::move(label), m_forKeyword, m_clause->clone(), m_body->clone());
        }
    private:
        const std::optional<LoopLabel> m_label;
        const Token m_forKeyword;
        const std::unique_ptr<const ForClause> m_clause;
        const std::unique_ptr<const Expression> m_body;
    };
}