#pragma once
#include <linc/system/Logger.hpp>
#include <linc/system/PrimitiveValue.hpp>
#include <linc/system/Reporting.hpp>
#include <linc/lexer/Token.hpp>
#include <linc/Include.hpp>

namespace linc
{
    template <typename IMPL>
    class BoundClause
    {
    public:
        BoundClause(const Token::Info& info)
            :m_info(info)
        {}
        [[nodiscard]] inline const Token::Info& getInfo() const { return m_info; }
        virtual ~BoundClause() = default;
        virtual std::unique_ptr<const IMPL> clone() const = 0;
    private:
        const Token::Info m_info;
    };
}