#pragma once
#include <linc/tree/Expression.hpp>

namespace linc
{
    class TypeExpression;
    class DefaultExpression final : public Expression
    {
    public:
        DefaultExpression(const Token& default_keyword, const Token& left_angled_bracket, const Token& right_angled_bracket,
            std::unique_ptr<const TypeExpression> type);
        ~DefaultExpression();
        [[nodiscard]] inline const TypeExpression* const getInnerType() const { return m_innerType.get(); }
        
        virtual std::unique_ptr<const Expression> clone() const final override;
    private:
        const Token m_defaultKeyword, m_leftAngledBracket, m_rightAngledBracket;
        const std::unique_ptr<const TypeExpression> m_innerType;
    };
}