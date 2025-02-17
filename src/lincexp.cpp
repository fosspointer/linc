#include <linc/ControlFlow.hpp>
// #include <linc/Generator.hpp>
#include <linc/Binder.hpp>
#include <linc/BoundTree.hpp>
#include <linc/Tree.hpp>
#include <linc/Parser.hpp>
#include <linc/Preprocessor.hpp>
#include <linc/Lexer.hpp>
#include <linc/System.hpp>

#define LINC_EXIT_SUCCESS 0
#define LINC_EXIT_FAILURE_LINC_EXCEPTION 1
#define LINC_EXIT_FAILURE_STANDARD_EXCEPTION 2
#define LINC_EXIT_FAILURE_UNKNOWN_EXCEPTION 3
#define LINC_EXIT_COMPILATION_FAILURE 4

using namespace linc;
class TestInterpreter final
{
public:
    struct EdgeMover
    {
        void operator()(linc::ControlBlock& block) { *block_pointer = block.edge; }
        void operator()(linc::BasicBlock& block) { *block_pointer = block.edge; }
        void operator()(linc::UnreachableBlock& block) { throw LINC_EXCEPTION_ILLEGAL_STATE(*block_pointer); }
        void operator()(linc::ConditionalBlock& block) { throw LINC_EXCEPTION_ILLEGAL_STATE(*block_pointer); }
        void operator()(linc::MapBlock& block) { throw LINC_EXCEPTION_ILLEGAL_STATE(*block_pointer); }
        std::size_t* block_pointer;
    };

    struct Blocker
    {
        void operator()(linc::ControlBlock& block) { *block_pointer = block.edge; }
        void operator()(linc::BasicBlock& block)
        {
            for(const auto& statement: block.statements)
                interpreter.evaluateStatement(statement.get());
            *block_pointer = block.edge;
        }
        void operator()(linc::UnreachableBlock& block) { *block_pointer = -1ul; }
        void operator()(linc::ConditionalBlock& block)
        {
            auto condition = interpreter.evaluateExpression(block.condition.get());
            if(condition.getPrimitive().getBool())
                *block_pointer = block.edgeTrue;
            else *block_pointer = block.edgeFalse;
        }
        void operator()(linc::MapBlock& block)
        {
            auto test = interpreter.evaluateExpression(block.testExpression.get());
            bool matched{false};
            for(const auto& _case: block.cases)
            {
                interpreter.m_callStack.push(std::pair<std::size_t, std::size_t>(interpreter.m_callStack.top().first, _case.testIndex));
                interpreter.walkCurrentContext();
                auto candidate = interpreter.evaluateExpression(_case.candidateExpression.get());
                interpreter.m_callStack.pop();
                if(candidate == test)
                {
                    *block_pointer = _case.resultIndex;
                    matched = true;
                    break;
                }
            }
            if(!matched)
                *block_pointer = block.edgeExit;
        }
        std::size_t* block_pointer;
        TestInterpreter& interpreter;
    };
    
    int evaluateProgram(ControlFlowProgram&& program)
    {
        m_callStack = {};
        m_functions = {};
        m_variables = {};
        m_program = std::move(program);


        m_variables.resize(m_program.functions.size());
        for(const auto& variable: m_program.globals)
            m_variables[0ul].insert(std::make_pair(variable.first, Value::fromDefault(variable.second)));
        
        for(std::size_t i{1ul}; i < m_program.functions.size(); ++i)
            m_functions.insert(std::make_pair(m_program.functions[i].prototype->getName(), i));

        m_callStack.push(std::make_pair(m_program.functions.size() - 1ul, 0ul));
        walkCurrentContext();
        return LINC_EXIT_SUCCESS;
    }

    void walkCurrentContext()
    {
        static Blocker blocker{nullptr, *this};
        while(m_callStack.top().second != -1ul)
        {
            auto& [function_pointer, block_pointer] = m_callStack.top();
            blocker.block_pointer = &block_pointer;
            std::visit(blocker, m_program.functions[m_callStack.top().first].blocks[m_callStack.top().second]);
        }
    }

    void evaluateDeclaration(const BoundDeclaration* declaration)
    {
        if(auto variable_declaration = dynamic_cast<const BoundVariableDeclaration*>(declaration))
            m_variables[m_callStack.top().first].insert_or_assign(variable_declaration->getName(), 
                variable_declaration->getDefaultValue()? evaluateExpression(variable_declaration->getDefaultValue()):
                    Value::fromDefault(variable_declaration->getActualType())
            );
    }

    Value evaluateExpression(const BoundExpression* expression)
    {
        if(auto identifier_expression = dynamic_cast<const BoundIdentifierExpression*>(expression))
        {
            auto find = m_variables[m_callStack.top().first].find(identifier_expression->getValue());
            if(find != m_variables[m_callStack.top().first].end()) return find->second;
            auto actual_name = identifier_expression->getValue();
            actual_name = actual_name.substr(0ul, actual_name.find_first_of(':'));
            auto& function = m_program.functions[m_functions.at(actual_name)];
            return PrimitiveValue(function.prototype->getName());
        }

        else if(auto literal_expression = dynamic_cast<const BoundLiteralExpression*>(expression))
            return literal_expression->getValue();

        else if(auto external_call = dynamic_cast<const BoundExternalCallExpression*>(expression))
        {
            const auto& name = external_call->getName();
            
            if(name == "puts")
            {
                fputs(evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive().getString().c_str(), stdout);
                return PrimitiveValue::voidValue;
            }
            else if(name == "putln")
            {
                fputs((evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive().getString() + '\n').c_str(), stdout);
                return PrimitiveValue::voidValue;
            }
            else if(name == "putc")
            {
                fputc(evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive().getChar(), stdout);
                return PrimitiveValue::voidValue;
            }
            else if(name == "readc")
            {
                return PrimitiveValue(static_cast<char>(std::getchar()));
            }
            else if(name == "readraw")
            {
                std::string result;
                std::getline(std::cin, result);
                return linc::PrimitiveValue(result);
            }
            else if(name == "readln")
            {
                auto prompt = evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive().getString();
                return linc::PrimitiveValue(Logger::read(prompt));
            }
            else if(name == "sys_write")
            {
            #ifdef LINC_LINUX                        
                auto file_descriptor = evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive();
                auto string = evaluateExpression(external_call->getArguments().at(1ul).get()).getPrimitive();
                auto size = evaluateExpression(external_call->getArguments().at(2ul).get()).getPrimitive();
                auto result = syscall(SYS_write, file_descriptor.getI32(), string.getString().c_str(), size.getU64());
                return PrimitiveValue(result < 0? -errno: result);

            #else
                return PrimitiveValue::invalidValue;
            #endif
            }
            else if(name == "sys_exit")
            {
            #ifdef LINC_LINUX                    
                auto arg_0 = evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive();
                auto result = syscall(SYS_exit, arg_0.getI32());
                return PrimitiveValue(result < 0? -errno: result);
            #else
                return PrimitiveValue::invalidValue;
            #endif
            }
            else if(name == "sys_open")
            {
            #ifdef LINC_LINUX
                auto filename = evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive().getString();
                auto flags = evaluateExpression(external_call->getArguments().at(1ul).get()).getPrimitive().getI32();
                auto mode = evaluateExpression(external_call->getArguments().at(2ul).get()).getPrimitive().getU16();

                auto result = syscall(SYS_open, filename.c_str(), flags, mode);
                return PrimitiveValue(result < 0? -errno: result);
            #else
                return PrimitiveValue::invalidValue;
            #endif
            }
            else if(name == "sys_read")
            {
            #ifdef LINC_LINUX
                auto identifier = dynamic_cast<const BoundIdentifierExpression*>(external_call->getArguments().at(1ul).get());
                if(!identifier)
                    return (Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                        .message = "String argument in sys_read must be an identifier"
                    }), PrimitiveValue::invalidValue);

                auto file = evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive().getI32();
                auto buffer = evaluateExpression(external_call->getArguments().at(1ul).get()).getPrimitive().getString();
                auto count = evaluateExpression(external_call->getArguments().at(2ul).get()).getPrimitive().getU64();
                buffer.resize(count);

                auto result = syscall(SYS_read, file, &buffer[0ul], count);

                m_variables[m_callStack.top().first].at(identifier->getValue()) = PrimitiveValue(buffer);
                return PrimitiveValue(result < 0? -errno: result);
            #else
                return PrimitiveValue::invalidValue;
            #endif
            }
            else if(name == "sys_close")
            {
            #ifdef LINC_LINUX
                auto file_descriptor = evaluateExpression(external_call->getArguments().at(0ul).get()).getPrimitive().getI32();
                auto result = syscall(SYS_close, file_descriptor);
                return PrimitiveValue(result < 0? -errno: result);
            #else
                return PrimitiveValue::invalidValue;
            #endif
            }
            else if(name == "system")
            {
                class Deleter
                {
                public:
                    inline void operator()(std::FILE* file){ pclose(file); }
                };

                static std::array<char, 128ul> buffer;
                auto argument = evaluateExpression(external_call->getArguments().at(0ul).get());

                std::string result, command = argument.getPrimitive().getString();

                std::unique_ptr<std::FILE, Deleter> pipe(popen(command.c_str(), "r"));
                
                if(!pipe)
                    return (Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                        .message = Logger::format("Failed to open pipe while evaluating shell expression (with command '$').", command)
                    }), PrimitiveValue::invalidValue);

                while(std::fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) result += buffer.data();
                return PrimitiveValue(result);
            }
            
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                .message = Logger::format("Internal function `$` has not been implemented.", name)
            });

            return PrimitiveValue::invalidValue;
        }

        else if(auto binary_expression = dynamic_cast<const BoundBinaryExpression*>(expression))
        {
            Value result = Value::fromDefault(binary_expression->getType());
            
            if(binary_expression->getOperator()->getKind() == BoundBinaryOperator::Kind::LogicalAnd)
            {
                if(evaluateExpression(binary_expression->getLeft()).getPrimitive().getBool())
                    return PrimitiveValue(evaluateExpression(binary_expression->getRight()).getPrimitive().getBool());
                else return PrimitiveValue(false);
            }
            else if(binary_expression->getOperator()->getKind() == BoundBinaryOperator::Kind::LogicalOr)
            {
                if(evaluateExpression(binary_expression->getLeft()).getPrimitive().getBool())
                    return PrimitiveValue(true);
                else return PrimitiveValue(evaluateExpression(binary_expression->getRight()).getPrimitive().getBool());
            }
            else if(binary_expression->getOperator()->getKind() == BoundBinaryOperator::Kind::Addition)
            {
                auto left = evaluateExpression(binary_expression->getLeft());
                auto right = evaluateExpression(binary_expression->getRight());

                result = left + right;
            }

            auto left = evaluateExpression(binary_expression->getLeft());
            auto right = evaluateExpression(binary_expression->getRight());

            switch(binary_expression->getOperator()->getKind())
            {
            case BoundBinaryOperator::Kind::Assignment:
                return evaluateMutableOperator(binary_expression->getType(), binary_expression->getLeft(), right);
            case BoundBinaryOperator::Kind::AdditionAssignment:
                return evaluateMutableOperator(binary_expression->getType(), binary_expression->getLeft(), left + right);
            case BoundBinaryOperator::Kind::SubtractionAssignment:
                return evaluateMutableOperator(binary_expression->getType(), binary_expression->getLeft(), left.getPrimitive() - right.getPrimitive());
            case BoundBinaryOperator::Kind::MultiplicationAssignment:
                return evaluateMutableOperator(binary_expression->getType(), binary_expression->getLeft(), left.getPrimitive() * right.getPrimitive());
            case BoundBinaryOperator::Kind::DivisionAssignment:
                return evaluateMutableOperator(binary_expression->getType(), binary_expression->getLeft(), left.getPrimitive() / right.getPrimitive());
            case BoundBinaryOperator::Kind::ModuloAssignment:
                return evaluateMutableOperator(binary_expression->getType(), binary_expression->getLeft(), left.getPrimitive() % right.getPrimitive());
            case BoundBinaryOperator::Kind::Subtraction:
                result = left - right;
                break;
            case BoundBinaryOperator::Kind::Multiplication:
                result = left * right;
                break;
            case BoundBinaryOperator::Kind::Division:
                if(right.getPrimitive().isZero() && !Types::isFloating(binary_expression->getOperator()->getReturnType().primitive))
                    return (Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                        .message = Logger::format("Attempted division by zero. Operands are `$` and `$` (`$`).",
                            left, right, binary_expression->getLeft()->getType())
                    }), PrimitiveValue::invalidValue);
                result = left / right;
                break;
            case BoundBinaryOperator::Kind::Modulo:
                if(right.getPrimitive().isZero())
                    return (Reporting::push(Reporting::Report{
                        .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                        .message = Logger::format("Attempted modulo division by zero. Operands are `$` and `$` (`$`).",
                            left, right, binary_expression->getLeft()->getType())
                    }), PrimitiveValue::invalidValue);
                result = left % right;
                break;
            case BoundBinaryOperator::Kind::Equals:
                result = PrimitiveValue(left == right);
                break;
            case BoundBinaryOperator::Kind::NotEquals:
                result = PrimitiveValue(left != right);
                break;
            case BoundBinaryOperator::Kind::Greater:
                result = PrimitiveValue(left > right);
                break;
            case BoundBinaryOperator::Kind::Less:
                result = PrimitiveValue(left < right);
                break;
            case BoundBinaryOperator::Kind::GreaterEqual:
                result = PrimitiveValue(left >= right);
                break;
            case BoundBinaryOperator::Kind::LessEqual:
                result = PrimitiveValue(left <= right);
                break;
            case BoundBinaryOperator::Kind::BitwiseAnd:
                result = left & right;
                break;
            case BoundBinaryOperator::Kind::BitwiseOr:
                result = left | right;
                break;
            case BoundBinaryOperator::Kind::BitwiseXor:
                result = left ^ right;
                break;
            case BoundBinaryOperator::Kind::BitwiseShiftLeft:
                result = left << right;
                break;
            case BoundBinaryOperator::Kind::BitwiseShiftRight:
                result = left >> right;
                break;
            default: break;
            }

            if(result.getIfPrimitive() && (result.getPrimitive().getKind() == PrimitiveValue::Kind::Signed
                || result.getPrimitive().getKind() == PrimitiveValue::Kind::Unsigned))
                return result.getPrimitive().convert(binary_expression->getType().primitive);
            else return result;
        }
        else if(auto unary_expression = dynamic_cast<const BoundUnaryExpression*>(expression))
        {
            Value result = Value::fromDefault(unary_expression->getType());
            
            if(unary_expression->getOperator()->getKind() == BoundUnaryOperator::Kind::Typeof)
                return PrimitiveValue(unary_expression->getOperand()->getType());

            auto operand = evaluateExpression(unary_expression->getOperand());

            switch(unary_expression->getOperator()->getKind())
            {
            case BoundUnaryOperator::Kind::Increment:
                result = evaluateMutableOperator(unary_expression->getType(), unary_expression->getOperand(),
                    operand + PrimitiveValue{1}.convert(unary_expression->getType().primitive));
                break;
            case BoundUnaryOperator::Kind::Decrement:
                result = evaluateMutableOperator(unary_expression->getType(), unary_expression->getOperand(), 
                    operand - PrimitiveValue{1}.convert(unary_expression->getType().primitive));
                break;
            case BoundUnaryOperator::Kind::Stringify:
                result = PrimitiveValue(operand.toApplicationString());
                break;
            case BoundUnaryOperator::Kind::UnaryPlus:
                if(operand.getIfArray())
                    result = PrimitiveValue(operand.getArray().getCount());
                else if(operand.getPrimitive().getKind() == PrimitiveValue::Kind::String)
                    result = PrimitiveValue(static_cast<Types::u64>(operand.getPrimitive().getString().size()));
                else if(operand.getPrimitive().getKind() == PrimitiveValue::Kind::Character)
                    result = PrimitiveValue(+operand.getPrimitive().getChar());
                else if(operand.getPrimitive().getKind() == PrimitiveValue::Kind::Boolean)
                    result = PrimitiveValue(static_cast<Types::i32>(operand.getPrimitive().getBool()));
                else result = operand;
                break;
            case BoundUnaryOperator::Kind::UnaryMinus:
                if(auto structure = operand.getIfStructure())
                {
                    structure->at(2ul) = PrimitiveValue{!structure->at(2ul).getPrimitive().getBool()};
                    result = Value{*structure};
                    break;
                }
                else result = -operand;
                break;
            case BoundUnaryOperator::Kind::LogicalNot:
                result = PrimitiveValue(!operand.getPrimitive().getBool());
                break;
            case BoundUnaryOperator::Kind::BitwiseNot:
                result = ~operand;
                break;
            default: 
                result = PrimitiveValue::invalidValue;
            }

            if(result.getIfPrimitive() && (result.getPrimitive().getKind() == PrimitiveValue::Kind::Signed
                || result.getPrimitive().getKind() == PrimitiveValue::Kind::Unsigned))
                return result.getPrimitive().convert(unary_expression->getType().primitive);
            else return result;
        }

        else if(auto function_call_expression = dynamic_cast<const BoundFunctionCallExpression*>(expression))
            evaluateFunctionCallExpression(function_call_expression);

        return PrimitiveValue::voidValue;
    }

    Value evaluateFunctionCallExpression(const BoundFunctionCallExpression* expression)
    {
        auto function = evaluateExpression(expression->getFunction());
        auto function_name = function.getPrimitive().getString();
        auto function_index = m_functions.at(function_name);
        for(std::size_t i{0ul}; i < expression->getArguments().size(); ++i)
        {
            const auto& argument = expression->getArguments()[i];
            auto value = evaluateExpression(argument.get());
            auto name = m_program.functions.at(function_index).prototype->getArguments()->getList()[i]->getName();
            m_variables[function_index].insert_or_assign(name, value);
        }
        m_callStack.push(std::make_pair(function_index, 0ul));
        walkCurrentContext();
        auto result = evaluateExpression(m_program.functions[m_callStack.top().first].returnValue.get());
        m_callStack.pop();
        static EdgeMover mover;
        mover.block_pointer = &m_callStack.top().second;
        std::visit(mover, m_program.functions[m_callStack.top().first].blocks[m_callStack.top().second]);
        return result;
    }

    void evaluateStatement(const BoundStatement* statement)
    {
        if(auto expression = dynamic_cast<const BoundExpressionStatement*>(statement))
            evaluateExpression(expression->getExpression());

        else if(auto declaration_statement = dynamic_cast<const BoundDeclarationStatement*>(statement))
            evaluateDeclaration(declaration_statement->getDeclaration());
    }

    Value evaluateMutableOperator(const Types::type& type, const BoundExpression* expression, const Value& new_value)
    {
        Value result = Value::fromDefault(type);

        if(auto identifier = dynamic_cast<const BoundIdentifierExpression*>(expression))
        {
            m_variables[m_callStack.top().first].at(identifier->getValue()) = new_value;
            result = new_value;
        }
        else if(auto index_expression = dynamic_cast<const BoundIndexExpression*>(expression))
        {
            auto index = evaluateExpression(index_expression->getIndex()).getPrimitive().getU64();
            auto array = index_expression->getArray();

            if(auto identifier = dynamic_cast<const BoundIdentifierExpression*>(array))
            {
                auto find = m_variables[m_callStack.top().first].find(identifier->getValue());
                
                if(find == m_variables[m_callStack.top().first].end())
                    return PrimitiveValue::invalidValue;

                find->second.getArray().set(index, new_value);
                result = new_value;
                return result;
            }
            
            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                .message = "Cannot use mutable operator on temporary array operands."
            });

            return PrimitiveValue::invalidValue;
        }
        else if(auto access_expression = dynamic_cast<const BoundAccessExpression*>(expression))
        {
            auto index = access_expression->getIndex();
            auto base = access_expression->getBase();

            if(auto identifier = dynamic_cast<const BoundIdentifierExpression*>(base))
            {
                auto find = m_variables[m_callStack.top().first].find(identifier->getValue());
                
                if(find == m_variables[m_callStack.top().first].end())
                    return PrimitiveValue::invalidValue;

                find->second.getStructure().at(index) = new_value;
                result = new_value;
                return result;
            }

            Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                .message = "Cannot use mutable operator on temporary structure operands."
            });

            return PrimitiveValue::invalidValue;
        }
        else return (Reporting::push(Reporting::Report{
                .type = Reporting::Type::Error, .stage = Reporting::Stage::Generator,
                .message = "Cannot use mutable operator on temporary operands."
            }), PrimitiveValue::invalidValue);

        if(result.getIfPrimitive() && (result.getPrimitive().getKind() == PrimitiveValue::Kind::Signed
            || result.getPrimitive().getKind() == PrimitiveValue::Kind::Unsigned))
            return result.getPrimitive().convert(type.primitive);
        else return result;
    }

    std::vector<std::unordered_map<std::string, Value>> m_variables;
    std::unordered_map<std::string, std::size_t> m_functions;
    std::stack<std::pair<std::size_t, std::size_t>> m_callStack;
    ControlFlowProgram m_program;
};

static int evaluateFile(std::string filepath)
{
    filepath = linc::Files::toAbsolute(filepath);

    if(!linc::Files::exists(filepath))
    {
        linc::Reporting::push(linc::Reporting::Report{
            .type = linc::Reporting::Type::Error, .stage = linc::Reporting::Stage::Environment,
            .message = linc::Logger::format("Specified filepath `$` does not exist!", filepath)
        });
        std::exit(LINC_EXIT_COMPILATION_FAILURE);
    }
    auto raw_code = linc::Files::read(filepath);
    auto code = linc::Code::toSource(raw_code, filepath);

    linc::Lexer lexer(code, true);
    auto tokens = lexer();

    linc::Preprocessor preprocessor;
    preprocessor.set(tokens, filepath);
    auto processed_code = preprocessor();

    linc::Parser parser;
    parser.set(processed_code, filepath);
    linc::Binder binder;
    
    auto program = parser();
    auto bound_program = binder.bindProgram(&program);
    
    if(linc::Reporting::hasError())
        std::exit(LINC_EXIT_COMPILATION_FAILURE);

    linc::Lowerer lowerer;
    auto control_flow_program = lowerer.lowerProgram(bound_program, binder);
    TestInterpreter interpreter;
    return interpreter.evaluateProgram(std::move(control_flow_program));
}

int main(int argc, char** argv)
try
{
    if(argc != 2)
    {
        std::fputs("No file specified\n", stderr);
        std::exit(LINC_EXIT_COMPILATION_FAILURE);
    }
    return evaluateFile(argv[1]);
}
catch(linc::Exception& exception)
{

    linc::Logger::log(linc::Logger::Type::Error, "LINC EXCEPTION $", exception.info());
}
catch(std::exception& exception)
{
    linc::Logger::log(linc::Logger::Type::Error, "STANDARD EXCEPTION $", exception.what());
}
catch(...)
{
    linc::Logger::log(linc::Logger::Type::Error, "UNKNOWN EXCEPTION");
}