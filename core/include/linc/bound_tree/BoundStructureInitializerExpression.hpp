#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundStructureInitializerExpression final : public BoundExpression
    {
    public:
        BoundStructureInitializerExpression(const std::string& name, std::vector<std::unique_ptr<const BoundExpression>> fields, const Types::type& type)
            :BoundExpression(type), m_name(name), m_fields(std::move(fields))
        {}

        virtual std::unique_ptr<const BoundExpression> clone() const final override
        {
            std::vector<std::unique_ptr<const BoundExpression>> fields;
            fields.reserve(m_fields.size());

            for(const auto& field: m_fields)
                fields.push_back(field->clone());

            return std::make_unique<const BoundStructureInitializerExpression>(m_name, std::move(fields), getType());
        }

        [[nodiscard]] const std::string& getName() const { return m_name; }
        [[nodiscard]] const std::vector<std::unique_ptr<const BoundExpression>>& getFields() const { return m_fields; }
    private:
        virtual std::string toStringInner() const final override
        {
            return "Structure Initializer Expression";
        }
        const std::string m_name;
        const std::vector<std::unique_ptr<const BoundExpression>> m_fields;
    };
}