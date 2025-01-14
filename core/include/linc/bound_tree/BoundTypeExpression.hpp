#pragma once
#include <linc/bound_tree/BoundExpression.hpp>
#include <linc/system/Logger.hpp>
#include <linc/Include.hpp>

namespace linc
{
    class BoundTypeExpression final : public BoundExpression 
    {
    public:
        using BoundArraySpecifiers = std::vector<std::optional<Types::u64>>;
        using Root = std::variant<Types::type::Primitive, Types::type::Structure, Types::type::Function, Types::type::Enumeration>;
        BoundTypeExpression(Types::type::Primitive primitive, bool is_mutable, BoundArraySpecifiers specifiers);
        BoundTypeExpression(Types::type::Structure structure, bool is_mutable, BoundArraySpecifiers specifiers);
        BoundTypeExpression(Types::type::Enumeration structure, bool is_mutable, BoundArraySpecifiers specifiers);
        BoundTypeExpression(Types::type::Function function, bool is_mutable, BoundArraySpecifiers specifiers);

        [[nodiscard]] inline bool getMutable() const { return m_isMutable; }
        [[nodiscard]] inline bool getArray() const { return !m_arraySpecifiers.empty(); }
        [[nodiscard]] inline const BoundArraySpecifiers& getArraySpecifiers() const { return m_arraySpecifiers; }
        [[nodiscard]] inline const Root& getBase() const { return m_root; }
        [[nodiscard]] inline Types::type getActualType() const
        {
            std::unique_ptr<Types::type> root{nullptr};
            if(auto kind = std::get_if<Types::type::Primitive>(&m_root))
                root = std::make_unique<Types::type>(*kind, m_isMutable);
            else if(auto structure = std::get_if<Types::type::Structure>(&m_root))
                root = std::make_unique<Types::type>(Types::type::cloneStructure(*structure, m_isMutable), m_isMutable);
            else if(auto enumeration = std::get_if<Types::type::Enumeration>(&m_root))
                root = std::make_unique<Types::type>(*enumeration, m_isMutable);
            else root = std::make_unique<Types::type>(Types::type::cloneFunction(&std::get<Types::type::Function>(m_root)), m_isMutable);

            if(m_arraySpecifiers.empty())
                return *root;
            else
            {
                Types::type result(Types::type::Array{.baseType = root->clone(),
                    .count = m_arraySpecifiers.back()}, m_isMutable);
                auto cursor = &result;

                for(std::size_t i{m_arraySpecifiers.size()}; i != 1ul; --i)
                {
                    cursor->array.baseType = std::make_unique<Types::type>(Types::type::Array{
                        .baseType = i == 2ul? root->clone() :nullptr,
                        .count = m_arraySpecifiers[i - 2ul]
                    }, m_isMutable);
                    cursor = const_cast<Types::type*>(cursor->array.baseType.get());
                }

                return result;
            }
        }

        virtual std::unique_ptr<const BoundExpression> clone() const final override;
    private:
        virtual std::string toStringInner() const final override;
        const Root m_root;
        const bool m_isMutable;
        const BoundArraySpecifiers m_arraySpecifiers;
    };
}