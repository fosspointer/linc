#include <linc/tree/DefaultExpression.hpp>
#include <linc/tree/TypeExpression.hpp>

namespace linc
{
    DefaultExpression::DefaultExpression(const Token& default_keyword, const Token& left_angled_bracket, const Token& right_angled_bracket,
        std::unique_ptr<const TypeExpression> type)
        :m_defaultKeyword(default_keyword), m_leftAngledBracket(left_angled_bracket), m_rightAngledBracket(right_angled_bracket),
        m_innerType(std::move(type))
    {}
    DefaultExpression::~DefaultExpression() = default;  
    
    std::unique_ptr<const Expression> DefaultExpression::clone() const
    {
        auto inner_type = Memory::uniqueCast<const TypeExpression>(m_innerType->clone());
        return std::make_unique<const DefaultExpression>(m_defaultKeyword, m_leftAngledBracket, m_rightAngledBracket, std::move(inner_type));
    }
}