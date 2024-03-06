#pragma once
#include <linc/system/Exception.hpp>
#include <linc/Include.hpp>

#define LINC_TYPES_KIND_TYPE char
#define LINC_TYPE_BINARY_OPERATOR_INELIGIBLE_EXCEPTION(kind_literal) LINC_EXCEPTION("Binary " kind_literal " operator not eligible for 'type' operands.")

namespace linc
{   
    class Types final
    {
    public:
        Types() = delete;
        
        enum class Kind: LINC_TYPES_KIND_TYPE
        {
            invalid,
            u8, u16, u32, u64,
            i8, i16, i32, i64,
            f32, f64,
            string, type,
            _char, _bool, _void
        };

        struct type
        {
            Kind kind;
            bool isMutable{false};
            bool isArray{false};
            std::optional<std::size_t> arraySize{std::nullopt};

            bool operator==(const type& other) const
            {
                return kind == other.kind && isMutable == other.isMutable && isArray == other.isArray && arraySize == other.arraySize; 
            }

            bool operator!=(const type& other) const 
            {
                return !(*this == other);
            }

            bool isAssignableTo(const type& identifier_type) const
            {
                if(kind != identifier_type.kind || isArray != identifier_type.isArray) return false;
                else if(isArray)
                {
                    if((arraySize && identifier_type.arraySize && arraySize.value() == identifier_type.arraySize.value())
                    || !identifier_type.arraySize) return true;
                    else return false;
                }
                else return true;
            }

            bool isCompatible(const type& other) const
            {
                return isAssignableTo(other) || other.isAssignableTo(*this);
            }
        };
        
        using u8 = std::uint8_t;
        using u16 = std::uint16_t;
        using u32 = std::uint32_t;
        using u64 = std::uint64_t;

        using i8 = std::int8_t;
        using i16 = std::int16_t;
        using i32 = std::int32_t;
        using i64 = std::int64_t;

        using f32 = std::float32_t;
        using f64 = std::float64_t;

        using _char = char;
        using _bool = bool;

        using string = std::string;
        struct _void_type {};
        struct _invalid_type {};

        using Variant = std::variant<_invalid_type, _void_type, _bool, _char, u8, u16, u32, u64, i8, i16, i32, i64, f32, f64, string, type>;
        using TypeMap = std::unordered_map<std::string, Kind>;

        static type voidType, invalidType;
        
        template <typename To, typename From> 
        [[nodiscard]] inline static const std::unique_ptr<To> unique_cast_dynamic(std::unique_ptr<From> p)
        {
            std::unique_ptr<To> result(dynamic_cast<To*>(p.get()));
            p.release();
            return std::move(result);
        }

        template <typename To, typename From> 
        [[nodiscard]] inline static const std::unique_ptr<To> unique_cast(std::unique_ptr<From> p)
        {
            std::unique_ptr<To> result(static_cast<To*>(p.get()));
            p.release();
            return std::move(result);
        }

        static Variant toVariant(Kind type, const std::string& value);
        static std::string kindToString(Kind kind);
        static std::string toString(const type& type);
        static type fromKind(Kind kind);
        static Kind kindFromString(const std::string& value);
        static Kind kindFromUserString(const std::string& value);
        static Kind kindFromUserStringSuffix(const std::string& value);
        static Types::_bool parseBoolean(const std::string& str);
        static Types::_char parseUserCharacter(const std::string& str);
        static bool isNumeric(Kind type);
        static bool isIntegral(Kind type);
        static bool isSigned(Kind type);
        static bool isUnsigned(Kind type);
        static bool isFloating(Kind type);
    private:
        static const TypeMap s_typeMap, s_suffixMap;
    };
}