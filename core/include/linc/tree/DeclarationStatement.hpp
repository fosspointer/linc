#pragma once
#include <linc/tree/Declaration.hpp>
#include <linc/tree/Statement.hpp>

namespace linc
{
    class DeclarationStatement final : public Statement
    {
    public:
        DeclarationStatement(std::unique_ptr<const Declaration> declaration)
            :Statement(declaration->getInfo()), m_declaration(std::move(declaration))
        {}

        virtual std::vector<const Node*> getChildren() const final override
        {
            return {m_declaration.get()};
        }

        virtual std::unique_ptr<const Statement> cloneConst() const final override
        {
            return std::make_unique<const DeclarationStatement>(std::move(m_declaration->cloneConst()));
        }

        inline const Declaration* getDeclaration() const { return m_declaration.get(); }
    private:
        const std::unique_ptr<const Declaration> m_declaration;
    };
}