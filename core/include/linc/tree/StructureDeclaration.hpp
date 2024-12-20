#pragma once
#include <linc/tree/Declaration.hpp>
#include <linc/tree/VariableDeclaration.hpp>
#include <linc/tree/IdentifierExpression.hpp>
#include <linc/system/Types.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class StructureDeclaration final : public Declaration
    {
    public:
        StructureDeclaration(const Token& structure_keyword, const Token& left_brace, const Token& right_brace,
            std::unique_ptr<const IdentifierExpression> identifier, std::vector<std::unique_ptr<const VariableDeclaration>> fields)
            :Declaration(structure_keyword.info), m_structureKeyword(structure_keyword), m_leftBrace(left_brace), m_rightBrace(right_brace),
            m_identifier(std::move(identifier)), m_fields(std::move(fields))
        {
            addToken(m_structureKeyword);
            addTokens(m_identifier->getTokens());
            addToken(m_leftBrace);

            for(const auto& field: m_fields)
                addTokens(field->getTokens());

            addToken(m_rightBrace);
        }

        std::unique_ptr<const Declaration> clone() const final override
        {
            std::vector<std::unique_ptr<const VariableDeclaration>> fields;
            fields.reserve(m_fields.size());

            for(const auto& field: m_fields)
            {
                auto new_field = Types::uniqueCast<const VariableDeclaration>(field->clone());
                fields.push_back(std::move(new_field));
            }

            auto identifier = Types::uniqueCast<const IdentifierExpression>(m_identifier->clone());

            return std::make_unique<const StructureDeclaration>(m_structureKeyword, m_leftBrace, m_rightBrace, std::move(identifier),  std::move(fields));
        }

        inline const Token& getStructureKeyword() const { return m_structureKeyword; }
        inline const Token& getLeftBrace() const { return m_leftBrace; }
        inline const Token& getRightBrace() const { return m_rightBrace; }
        inline const IdentifierExpression* const getIdentifier() const { return m_identifier.get(); }
        inline const std::vector<std::unique_ptr<const VariableDeclaration>>& getFields() const { return m_fields; }
    private:
        const Token m_structureKeyword, m_leftBrace, m_rightBrace;
        const std::unique_ptr<const IdentifierExpression> m_identifier;
        const std::vector<std::unique_ptr<const VariableDeclaration>> m_fields;
    };
}