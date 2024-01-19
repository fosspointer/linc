#pragma once
#include <linc/bound_tree/BoundDeclaration.hpp>
#include <linc/bound_tree/BoundStatement.hpp>
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundArgumentDeclaration final : public BoundDeclaration 
    {
    public:
        BoundArgumentDeclaration(Types::Type type, const std::string& name, bool is_mutable, 
            std::optional<std::unique_ptr<const BoundExpression>> default_value)
            :BoundDeclaration(type), m_name(name), m_isMutable(is_mutable), m_defaultValue(std::move(default_value))
        {}

        [[nodiscard]] inline const std::string& getName() const { return m_name; }
        [[nodiscard]] inline bool getMutable() const { return m_isMutable; }
        [[nodiscard]] inline std::optional<const BoundExpression*> getDefaultValue() const
        {
            if(m_defaultValue.has_value())
                return m_defaultValue.value().get();
            else return std::nullopt;
        }

        virtual std::unique_ptr<const BoundDeclaration> clone_const() const final override
        {
            if(m_defaultValue.has_value())
                return std::make_unique<const BoundArgumentDeclaration>(getType(), m_name, m_isMutable, 
                    std::move(m_defaultValue.value()->clone_const()));
            else return std::make_unique<const BoundArgumentDeclaration>(getType(), m_name, m_isMutable, std::nullopt);
        }
    private:
        virtual std::string toStringInner() const final override
        {
            return "Bound Argument Declaration";
        }

        std::string m_name;
        bool m_isMutable;
        std::optional<std::unique_ptr<const BoundExpression>> m_defaultValue;
    };

    class BoundFunctionDeclaration final : public BoundDeclaration
    {
    public:
        BoundFunctionDeclaration(Types::Type type, const std::string& name, std::vector<std::unique_ptr<const BoundArgumentDeclaration>> arguments, 
            std::unique_ptr<const BoundStatement> body);

        [[nodiscard]] inline Types::Type getReturnType() const { return m_returnType; }
        [[nodiscard]] inline const std::string& getName() const { return m_name; }
        [[nodiscard]] inline const std::vector<std::unique_ptr<const BoundArgumentDeclaration>>& getArguments() const { return m_arguments; }
        [[nodiscard]] inline const BoundStatement* const getBody() const { return m_body.get(); }

        virtual std::unique_ptr<const BoundDeclaration> clone_const() const final override;
    private:
        virtual std::string toStringInner() const final override;

        const Types::Type m_returnType;
        const std::string m_name;
        const std::vector<std::unique_ptr<const BoundArgumentDeclaration>> m_arguments;
        const std::unique_ptr<const BoundStatement> m_body;
    };
}