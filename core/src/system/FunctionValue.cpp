#include <linc/system/FunctionValue.hpp>
#include <linc/system/Value.hpp>

namespace linc
{
    FunctionValue::FunctionValue(const std::string& name, std::vector<std::string> arguments, std::unique_ptr<const BoundExpression> body)
        :m_name(name), m_argumentNames(arguments), m_body(std::move(body))
    {}

    FunctionValue::FunctionValue(const FunctionValue& other)
        :m_name(other.m_name), m_argumentNames(other.m_argumentNames), m_body(other.m_body->clone())
    {}

    FunctionValue::FunctionValue(FunctionValue&& other)
        :m_name(std::move(other.m_name)), m_argumentNames(std::move(other.m_argumentNames)), m_body(std::move(other.m_body))
    {}

    FunctionValue& FunctionValue::operator=(const FunctionValue& other)
    {
        m_name = other.m_name;
        m_argumentNames = other.m_argumentNames;
        m_body = other.m_body->clone();
        return *this;
    }

    FunctionValue& FunctionValue::operator=(FunctionValue&& other)
    {
        m_name = std::move(other.m_name);
        m_argumentNames = std::move(other.m_argumentNames);
        m_body = std::move(other.m_body);
        return *this;
    }

    bool FunctionValue::operator==(const FunctionValue& other) const
    {
        return m_body == other.m_body;
    }
}