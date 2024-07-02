#include <linc/bound_tree/BoundForExpression.hpp>

namespace linc
{
    BoundForExpression::BoundForExpression(std::unique_ptr<const BoundVariableDeclaration> declaration, std::unique_ptr<const BoundExpression> expression,
        std::unique_ptr<const BoundStatement> statement, std::unique_ptr<const BoundStatement> body)
        :BoundExpression(body->getType()), m_specifier(BoundVariableForSpecifier{std::move(declaration), std::move(expression), std::move(statement)}), 
        m_body(std::move(body))    
    {}

    BoundForExpression::BoundForExpression(std::unique_ptr<const BoundIdentifierExpression> value_identifier,
        std::unique_ptr<const BoundIdentifierExpression> array_identifier, std::unique_ptr<const BoundStatement> body)
        :BoundExpression(body->getType()), m_specifier(BoundRangeForSpecifier(std::move(value_identifier), std::move(array_identifier))),
        m_body(std::move(body))    
    {}

    std::unique_ptr<const BoundExpression> BoundForExpression::clone() const 
    {
        if(auto variable_for_specifier = std::get_if<const BoundVariableForSpecifier>(&m_specifier))
        {
            auto variable_declaration = Types::unique_cast<const BoundVariableDeclaration>(variable_for_specifier->variableDeclaration->clone());    

            return std::make_unique<const BoundForExpression>(
                std::move(variable_declaration), variable_for_specifier->expression->clone(),
                variable_for_specifier->statement->clone(), m_body->clone());
        }
        else if(auto range_for_specifier = std::get_if<const BoundRangeForSpecifier>(&m_specifier))
        {
            auto value_identifier = Types::unique_cast<const BoundIdentifierExpression>(range_for_specifier->valueIdentifier->clone());
            auto array_identifier = Types::unique_cast<const BoundIdentifierExpression>(range_for_specifier->arrayIdentifier->clone());

            return std::make_unique<const BoundForExpression>(std::move(value_identifier), std::move(array_identifier), m_body->clone());
        }
        else throw LINC_EXCEPTION_OUT_OF_BOUNDS((std::variant<const BoundVariableForSpecifier, const BoundRangeForSpecifier>));
    }

    std::string BoundForExpression::toStringInner() const 
    {
        return "For Expression";
    }
}