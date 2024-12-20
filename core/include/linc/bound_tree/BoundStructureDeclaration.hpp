#pragma once
#include <linc/bound_tree/BoundDeclaration.hpp>
#include <linc/bound_tree/BoundVariableDeclaration.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundStructureDeclaration final: public BoundDeclaration
    {
    public:
        BoundStructureDeclaration(const std::string& name, std::vector<std::unique_ptr<const BoundVariableDeclaration>> fields)
            :m_name(name), m_fields(std::move(fields))
        {}

        virtual std::unique_ptr<const BoundDeclaration> clone() const final override
        {
            std::vector<std::unique_ptr<const BoundVariableDeclaration>> fields;
            fields.reserve(m_fields.size());

            for(const auto& field: m_fields)
            {
                auto new_field = Types::uniqueCast<const BoundVariableDeclaration>(field->clone());
                fields.push_back(std::move(new_field));
            }

            return std::make_unique<const BoundStructureDeclaration>(m_name, std::move(fields));
        }

        Types::type getActualType() const
        {
            Types::type::Structure types;
            
            for(const auto& field: m_fields)
                types.push_back(std::pair(field->getName(), field->getActualType().clone()));

            return Types::type(std::move(types));
        }

        [[nodiscard]] inline const std::string& getName() const { return m_name; }
        [[nodiscard]] inline const std::vector<std::unique_ptr<const BoundVariableDeclaration>>& getFields() const { return m_fields; }
    private:
        virtual std::string toStringInner() const final override
        {
            return "Structure Declaration";
        }
        const std::string m_name;
        const std::vector<std::unique_ptr<const BoundVariableDeclaration>> m_fields;
    };
}