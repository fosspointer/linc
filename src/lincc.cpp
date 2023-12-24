#include <linc/System.hpp>
#include <linc/Lexer.hpp>
#include <linc/Parser.hpp>
#include <linc/Tree.hpp>
#include <linc/Binder.hpp>

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
    else if(auto* if_exp = dynamic_cast<const linc::IfElseExpression*>(&node))
        return "IfElseExpression";
    else if(auto* while_exp = dynamic_cast<const linc::WhileExpression*>(&node))
        return "WhileExpression";
    else if(auto* var_decl_stmt = dynamic_cast<const linc::VariableDeclarationStatement*>(&node))
        return linc::Logger::format("VariableDeclarationStatement: $", var_decl_stmt->getTypeNameIdentifierToken().value.value());
    else if(auto* paren_exp =  dynamic_cast<const linc::ParenthesisExpression*>(&node))
        return "ParenthesisExpression";
    else if(auto* exp_stmt = dynamic_cast<const linc::ExpressionStatement*>(&node))
        return "ExpressionStatement";
    else if(auto* scope_stmt =  dynamic_cast<const linc::ScopeStatement*>(&node))
        return "ScopeStatement";
    else if(auto* putc_stmt = dynamic_cast<const linc::PutCharacterStatement*>(&node))
        return "PutCharacterStatement";
    else if(auto* puts_stmt = dynamic_cast<const linc::PutStringStatement*>(&node))
        return "PutStringStatement";
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
            linc::TypedValue value = linc::TypedValue::voidValue;
            for(const auto& stmt: scopestmt->getStatements())
                value = evaluateStatement(stmt.get());
            
            return value; 
        }
        else if(auto* vardeclstmt = dynamic_cast<const linc::BoundVariableDeclarationStatement*>(statement))
        {
            auto value = evaluateExpression(vardeclstmt->getValueExpression());
            
            auto variable = m_variables[vardeclstmt->getName()] = Variable{
                .name = vardeclstmt->getName(),
                .value = value
            };

            return linc::TypedValue::voidValue;
        }
        else if(auto* putcstmt = dynamic_cast<const linc::BoundPutCharacterStatement*>(statement))
        {
            linc::Logger::print("$", evaluateExpression(putcstmt->getExpression()));
            return linc::TypedValue::voidValue;
        }
        else if(auto* putsstmt = dynamic_cast<const linc::BoundPutStringStatement*>(statement))
        {
            linc::Logger::print("$", evaluateExpression(putsstmt->getExpression()));
            return linc::TypedValue::voidValue;
        }
        else
        {
            throw LINC_EXCEPTION("Encountered unrecognized statement type while evaluating program"); 
            return linc::TypedValue::invalidValue;
        }
    }

    linc::TypedValue evaluateExpression(const linc::BoundExpression* expression)
    {
        if(auto* litexp = dynamic_cast<const linc::BoundLiteralExpression*>(expression))
        {
            return litexp->getValue();
        }
        else if(auto* ifexp = dynamic_cast<const linc::BoundIfElseExpression*>(expression))
        {
            auto test = evaluateExpression(ifexp->getTestExpression()).getBool();

            if(test)
                return evaluateStatement(ifexp->getBodyIfStatement());
            else if(ifexp->hasElse())
                return evaluateStatement(ifexp->getBodyElseStatement().value());
            else return linc::TypedValue::voidValue;
        }
        else if(auto* whilexp = dynamic_cast<const linc::BoundWhileExpression*>(expression))
        {
            auto test = evaluateExpression(whilexp->getTestExpression()).getBool();
            linc::TypedValue return_value = linc::TypedValue::voidValue;

            while(test)
                return_value = evaluateStatement(whilexp->getBodyStatement());
            
            return return_value;
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
                return left == right;
            case linc::BoundBinaryOperator::Kind::NotEquals:
                return left != right;
            case linc::BoundBinaryOperator::Kind::Greater:
                return left > right;
            case linc::BoundBinaryOperator::Kind::Less:
                return left < right;
            case linc::BoundBinaryOperator::Kind::GreaterEqual:
                return left >= right;
            case linc::BoundBinaryOperator::Kind::LessEqual:
                return left <= right;
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
            return linc::TypedValue::invalidValue;
        }
    }
private:
    std::unordered_map<std::string, Variable> m_variables;
};

int main(int argc, char** argv)
{
    try
    {
        if(argc >= 2)
        {
            std::string filename = argv[1];
            std::string file = linc::Files::read(filename);
            linc::Lexer lexer(file);
            linc::Parser parser(lexer());
            linc::Binder binder;
            auto tree = parser();
            auto program = binder.bindStatement(tree.get());
            Evaluator evaluator;
            evaluator.evaluateStatement(program.get());
            return LINC_EXIT_SUCCESS;
        }

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
            else if(buffer_tolower.starts_with("/vars"))
            {
                auto list = binder.getVariableList();
                
                for(size_t i = 0; i < list.size(); i++)
                {
                    const auto& var = list[i];
                    linc::Logger::println("#$:: var '$' of type '$'", i, var.first, linc::Types::toString(var.second));
                }
                
                if(list.empty())
                    linc::Logger::println("Variable index is empty");

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
                linc::Logger::println("/vars: displays the list of all declared variables.");
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
                    linc::Logger::println("Token {type: '$', value: '$'}", linc::Token::typeToString(token.type), token.value.value_or(""));

            linc::Parser parser(tokens);
            auto tree = parser();

            if(show_tree)
                printNodeTree(tree.get(), "");

            auto program = binder.bindStatement(tree.get());

            if(linc::Reporting::getReports().size() == 0)
            {
                auto res = evaluator.evaluateStatement(program.get());
                if(res.getType() != linc::Types::Type::invalid)
                    linc::Logger::println("-> $", res);
            }

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