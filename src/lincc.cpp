#include <linc/system/Files.hpp>
#include <linc/lexer/Lexer.hpp>
#include <linc/parser/Parser.hpp>
#include <linc/bound_tree/Binder.hpp>
#include <cstring>

#define LINC_EXIT_SUCCESS 0
#define LINC_EXIT_FAILURE_LINC_EXCEPTION 1
#define LINC_EXIT_FAILURE_STANDARD_EXCEPTION 2
#define LINC_EXIT_FAILURE_UNKNOWN_EXCEPTION 3

std::string nodeTypeToString(const linc::Node& node)
{
    if(auto* lit_exp = dynamic_cast<const linc::LiteralExpression*>(&node))
        return linc::Logger::format("LiteralExpression '$'", lit_exp->getValue());
    else if(auto* ident_exp = dynamic_cast<const linc::IdentifierExpression*>(&node))
        return linc::Logger::format("IdentifierExpression '$'", ident_exp->getIdentifierToken().value.value());
    else if(auto* un_exp = dynamic_cast<const linc::UnaryExpression*>(&node))
        return linc::Logger::format("UnaryExpression '$'", linc::Token::typeToString(un_exp->getOperatorToken().type));
    else if(auto* bin_exp = dynamic_cast<const linc::BinaryExpression*>(&node))
        return linc::Logger::format("BinaryExpression '$'", linc::Token::typeToString(bin_exp->getOperatorToken().type));
    else if(auto* var_decl_stmt = dynamic_cast<const linc::VariableDeclarationStatement*>(&node))
        return linc::Logger::format("VariableDeclarationStatement: $", var_decl_stmt->getTypeNameIdentifierToken().value.value());
    else if(auto* paren_exp =  dynamic_cast<const linc::ParenthesisExpression*>(&node))
        return "ParenthesisExpression";
    else if(auto* exp_stmt = dynamic_cast<const linc::ExpressionStatement*>(&node))
        return "ExpressionStatement";
    else if(auto* scope_stmt =  dynamic_cast<const linc::ScopeStatement*>(&node))
        return "ScopeStatement";
    else
        return "<unknown-node>";
}

void printNodeTree(const linc::Node* node, std::string indent = "", bool last = true)
{
    auto marker = last? "└──" : "├──";

    linc::Logger::println("$:$:$", indent, marker, nodeTypeToString(*node));

    indent += last? "   ": "│  ";

    const linc::Node* last_child = node->getChildren().size() > 0? node->getChildren()[node->getChildren().size() - 1]: nullptr;

    for(auto child: node->getChildren())
    {
        printNodeTree(child, indent, child == last_child);
    }
}

class Evaluator
{
public:
    struct Variable
    {   
        std::string name;
        linc::TypedValue value{false};
    };

    linc::TypedValue evaluateStatement(const linc::BoundStatement* statement)
    {
        if(auto* expstmt = dynamic_cast<const linc::BoundExpressionStatement*>(statement))
            return evaluateExpression(expstmt->getExpression());
        
        if(auto* scopestmt = dynamic_cast<const linc::BoundScopeStatement*>(statement))
        {
            linc::TypedValue value = false;
            for(const auto& stmt: scopestmt->getStatements())
                value = evaluateStatement(stmt);
            
            return value; 
        }
        else if(auto* vardeclstmt = dynamic_cast<const linc::BoundVariableDeclarationStatement*>(statement))
        {
            auto variable = m_variables[vardeclstmt->getName()] = Variable{
                .name = vardeclstmt->getName(),
                .value = evaluateExpression(vardeclstmt->getValueExpression()).convert(vardeclstmt->getType())
            };

            return variable.value;
        }
        else
        {
            throw LINC_EXCEPTION("Encountered unrecognized statement type while evaluating program"); 
            return 0;
        }
    }

    linc::TypedValue evaluateExpression(const linc::BoundExpression* expression)
    {
        if(auto* litexp = dynamic_cast<const linc::BoundLiteralExpression*>(expression))
        {
            return litexp->getValue();
        }
        else if(auto* binexp = dynamic_cast<const linc::BoundBinaryExpression*>(expression))
        {
            auto left = evaluateExpression(binexp->getLeft());
            auto right = evaluateExpression(binexp->getRight());

            switch(binexp->getOperator()->getKind())
            {
            case linc::BoundBinaryOperator::Kind::Addition:
                return left + right;
            case linc::BoundBinaryOperator::Kind::Subtraction:
                return left - right;
            case linc::BoundBinaryOperator::Kind::Multiplication:
                return left * right;
            case linc::BoundBinaryOperator::Kind::Division:
                return left / right;
            case linc::BoundBinaryOperator::Kind::LogicalAnd:
                return left.getBool() && right.getBool();
            case linc::BoundBinaryOperator::Kind::LogicalOr:
                return left.getBool() || right.getBool();
            case linc::BoundBinaryOperator::Kind::Equals:
                return bool(left == right);
            case linc::BoundBinaryOperator::Kind::NotEquals:
                return bool(left != right);
            default: return 0;
            }
        }
        else if(auto* unaryexp = dynamic_cast<const linc::BoundUnaryExpression*>(expression))
        {
            auto operand = evaluateExpression(unaryexp->getOperand());

            switch(unaryexp->getOperator()->getKind())
            {
            case linc::BoundUnaryOperator::Kind::UnaryPlus:
                return operand;
            case linc::BoundUnaryOperator::Kind::UnaryMinus:
                return -operand;
            case linc::BoundUnaryOperator::Kind::LogicalNot:
                return !operand.getBool();
            default: return 0;
            }
        }
        else if(auto* identexp = dynamic_cast<const linc::BoundIdentifierExpression*>(expression))
        {
            auto find = m_variables.find(identexp->getValue());
            
            if(find == m_variables.end())
            {
                linc::Reporting::push(linc::Reporting::Report{
                    .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Generator,
                    .message = linc::Logger::format("Variable '$' does not exist!", identexp->getValue())
                });

                return false;
            }

            return find->second.value;
        }
        else
        {
            throw LINC_EXCEPTION("Encountered unrecognized expression type while evaluating program"); 
            return -1;
        }
    }
private:
    std::unordered_map<std::string, Variable> m_variables;
};

int main(int argc, char** argv)
{
    try
    {
        bool show_tree{false}, show_lexer{false};

        linc::Binder binder;
        Evaluator evaluator;

        while(true)
        {
            linc::Logger::print("linc > ");

            std::string buffer, buffer_tolower;
            std::getline(std::cin, buffer);

            for(char& c: buffer)
                buffer_tolower.push_back(tolower(c));

            if(buffer_tolower.starts_with("/clear"))
            {
                system("clear");
                continue;
            }
            else if(buffer_tolower.starts_with("/tree"))
            {
                show_tree = !show_tree;
                linc::Logger::println("$ node tree display!", show_tree? "Enabled" : "Disabled");
                continue;
            }
            else if(buffer_tolower.starts_with("/lexer"))
            {
                show_lexer = !show_lexer;
                linc::Logger::println("$ lexer token display!", show_tree? "Enabled" : "Disabled");
                continue;
            }
            else if(buffer_tolower.starts_with("/reset"))
            {
                binder = linc::Binder();
                evaluator = Evaluator();
                system("clear");
                continue;
            }
            else if(buffer_tolower.starts_with("/file"))
            {
                std::string filename;

                linc::Logger::print("Enter the file to evaluate: ");
                std::getline(std::cin, filename);

                buffer = linc::Files::read(filename);
            }
            else if(buffer_tolower == "/?" || buffer_tolower == "/help" || buffer_tolower == "?")
            {
                linc::Logger::println("/clear: runs the GNU command of the same name.");
                linc::Logger::println("/tree: toggles the visual representation of the AST.");
                linc::Logger::println("/reset: gets rid of any declared variables.");
                linc::Logger::println("/lexer: toggles the lexer token display.");
                linc::Logger::println("/file: evaluate program from file.");
                linc::Logger::println("/help: displays this menu.");
                linc::Logger::println("/q: quits the application.");
                continue;
            }
            else if(buffer == "q" || buffer == "/q")
                return LINC_EXIT_SUCCESS;
            
            linc::Lexer lexer(buffer);
            auto tokens = lexer();

            if(show_lexer)
                for(auto& token: tokens)
                {
                    linc::Logger::println("Token {type: '$', value: '$'}", linc::Token::typeToString(token.type), token.value.value_or(""));
                }

            linc::Parser parser(tokens);
            auto tree = parser();

            if(show_tree)
                printNodeTree(tree, "");

            auto program = binder.bindStatement(tree);

            if(linc::Reporting::getReports().size() == 0)
                linc::Logger::println("-> $", evaluator.evaluateStatement(program));

            linc::Reporting::clearReports();
        }

        return LINC_EXIT_SUCCESS;
    }
    catch(const linc::Exception& e)
    {
        linc::Logger::log(linc::Logger::Type::Error, "[LINC EXCEPTION] $", e.info());
        return LINC_EXIT_FAILURE_LINC_EXCEPTION;
    }
    catch(const std::exception& e)
    {
        linc::Logger::log(linc::Logger::Type::Error, "[STANDARD EXCEPTION] $", e.what());
        return LINC_EXIT_FAILURE_STANDARD_EXCEPTION;
    }
    catch(...)
    {
        linc::Logger::log(linc::Logger::Type::Error, "[UNKNOWN EXCPETION]");
        return LINC_EXIT_FAILURE_UNKNOWN_EXCEPTION;
    }
}