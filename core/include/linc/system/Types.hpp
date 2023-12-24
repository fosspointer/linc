#pragma once
#include <cstdint>
#include <stdfloat>
#include <variant>
#include <unordered_map>
#include <linc/system/Exception.hpp>

namespace linc
{   
    class Types final
    {
    public:
        enum class Type: char
        {
            invalid,
            u8, u16, u32, u64,
            i8, i16, i32, i64,
            f32, f64,
            string,
            _char, _bool, _void
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

        using string = const _char*;
        struct _void_type {};
        struct invalid_type {};

        using Variant = std::variant<u8, u16, u32, u64, i8, i16, i32, i64, f32, f64, _char, _bool, _void_type, string>;

        using TypeMap = std::unordered_map<std::string, Type>;
        
        static Variant toVariant(Type type, const std::string& value);
        static std::string toString(Type type);
        static Type fromString(const std::string& value);
        static Type fromUserString(const std::string& value);
        static Type fromUserStringSuffix(const std::string& value);
        static Types::_bool parseBoolean(const std::string& str);
        static Types::_char parseCharacter(const std::string& str);
        static bool isNumeric(Type type);
        static bool isIntegral(Type type);
        static bool isSigned(Type type);
        static bool isUnsigned(Type type);
        static bool isFloating(Type type);
    private:
        static const TypeMap s_typeMap, s_suffixMap;
    };
}