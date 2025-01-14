#pragma once
#include <linc/system/Types.hpp>
#include <linc/bound_tree/BoundExpression.hpp>

namespace linc
{
    class FunctionValue final
    {
    public:
        FunctionValue(const std::string& name, std::vector<std::string> arguments, std::unique_ptr<const BoundExpression> body);
        FunctionValue(const FunctionValue& other);
        FunctionValue(FunctionValue&& other);
        FunctionValue& operator=(const FunctionValue& other);
        FunctionValue& operator=(FunctionValue&& other);
        [[nodiscard]] inline const std::string& getName() const { return m_name; }
        [[nodiscard]] inline const std::vector<std::string>& getArgumentNames() const { return m_argumentNames; }
        [[nodiscard]] inline const BoundExpression* const getBody() const { return m_body.get(); }
        bool operator==(const FunctionValue& other) const;
        bool operator!=(const FunctionValue& other) const = default;
    private:
        std::string m_name;
        std::vector<std::string> m_argumentNames;
        std::unique_ptr<const BoundExpression> m_body;
    };
}