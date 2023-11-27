#include <linc/system/Types.hpp>

#define LINC_TYPE_MAP_PAIR(first, second) std::pair<std::string, linc::Types::Type>(first, linc::Types::Type::second)

namespace linc
{
    const Types::TypeMap Types::s_typeMap = {
        LINC_TYPE_MAP_PAIR("u8", u8),
        LINC_TYPE_MAP_PAIR("u16", u16),
        LINC_TYPE_MAP_PAIR("u32", u32),
        LINC_TYPE_MAP_PAIR("u64", u64),

        LINC_TYPE_MAP_PAIR("i8", i8),
        LINC_TYPE_MAP_PAIR("i16", i16),
        LINC_TYPE_MAP_PAIR("i32", i32),
        LINC_TYPE_MAP_PAIR("i64", i64),
        LINC_TYPE_MAP_PAIR("i8", i8),

        LINC_TYPE_MAP_PAIR("f32", f32),
        LINC_TYPE_MAP_PAIR("f64", f64),

        LINC_TYPE_MAP_PAIR("char", _char),
        LINC_TYPE_MAP_PAIR("bool", _bool)
    };

    const Types::TypeMap Types::s_suffixMap = {
        LINC_TYPE_MAP_PAIR("u8", u8),
        LINC_TYPE_MAP_PAIR("u16", u16),
        LINC_TYPE_MAP_PAIR("u32", u32),
        LINC_TYPE_MAP_PAIR("u64", u64),
        LINC_TYPE_MAP_PAIR("u", u32),

        LINC_TYPE_MAP_PAIR("i8", i8),
        LINC_TYPE_MAP_PAIR("i16", i16),
        LINC_TYPE_MAP_PAIR("i32", i32),
        LINC_TYPE_MAP_PAIR("i64", i64),
        LINC_TYPE_MAP_PAIR("i", i32),

        LINC_TYPE_MAP_PAIR("f32", f32),
        LINC_TYPE_MAP_PAIR("f64", f64),
        LINC_TYPE_MAP_PAIR("f", f32),

        LINC_TYPE_MAP_PAIR("c", _char),
        LINC_TYPE_MAP_PAIR("b", _bool)
    };

    Types::Variant Types::toVariant(Type type, const std::string& value)
    {
        switch(type)
        {
        case Type::u8: return static_cast<u8>(std::stoul(value)); 
        case Type::u16: return static_cast<u16>(std::stoul(value));
        case Type::u32: return static_cast<u32>(std::stoul(value));
        case Type::u64: return static_cast<u64>(std::stoull(value));
        case Type::i8: return static_cast<i8>(std::stoi(value)); 
        case Type::i16: return static_cast<i16>(std::stoi(value));
        case Type::i32: return static_cast<i32>(std::stoi(value));
        case Type::i64: return static_cast<i64>(std::stoull(value));
        case Type::f32: return static_cast<f32>(std::stof(value));
        case Type::f64: return static_cast<f64>(std::stod(value));
        case Type::_char: return static_cast<_char>(value.at(0));
        case Type::_bool: return parseBoolean(value);
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Type);
        }
    }

    std::string Types::toString(Type type)
    {
        switch(type)
        {
        case Type::Invalid: return "<invalid_type>";
        case Type::u8: return "u8";
        case Type::u16: return "u16";
        case Type::u32: return "u32";
        case Type::u64: return "u64";
        case Type::i8: return "i8";
        case Type::i16: return "i16";
        case Type::i32: return "i32";
        case Type::i64: return "i64";
        case Type::f32: return "f32";
        case Type::f64: return "f64";
        case Type::_char: return "char";
        case Type::_bool: return "bool";
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::Type);
        }
    }

    Types::Type Types::fromString(const std::string& value)
    {
        auto f = s_typeMap.find(value);
        
        if(f != s_typeMap.end())
            return f->second;
        else throw LINC_EXCEPTION("String does not correspond do a type");
    }

    Types::Type Types::fromUserString(const std::string& value)
    {
        auto f = s_typeMap.find(value);
        
        if(f != s_typeMap.end())
            return f->second;
        else return Type::Invalid;
    }

    Types::Type Types::fromUserStringSuffix(const std::string& value)
    {
        auto f = s_typeMap.find(value);
        
        if(f != s_typeMap.end())
            return f->second;
        else return Type::Invalid;
    }

    Types::_bool Types::parseBoolean(const std::string& str)
    {
        if(str == "true")
            return true;
        else if(str == "false")
            return false;
        else
        {
            char* p;
            int64_t integral = strtoll(str.c_str(), &p, 10);
            
            if(*p)
                throw LINC_EXCEPTION_INVALID_INPUT("Invalid boolean expression");
            
            return integral != 0;
        }
    }

    bool Types::isNumeric(Type type)
    {
        switch(type)
        {
        case Type::i8:
        case Type::i16:
        case Type::i32:
        case Type::i64:
        case Type::u8:
        case Type::u16:
        case Type::u32:
        case Type::u64:
        case Type::f32:
        case Type::f64:
            return true;
        default: return false;
        }
    }

    bool Types::isIntegral(Type type)
    {
        switch(type)
        {
        case Type::i8:
        case Type::i16:
        case Type::i32:
        case Type::i64:
        case Type::u8:
        case Type::u16:
        case Type::u32:
        case Type::u64:
            return true;
        default: return false;
        }
    }
    
    bool Types::isSigned(Type type)
    {
        switch(type)
        {
        case Type::i8:
        case Type::i16:
        case Type::i32:
        case Type::i64:
            return true;
        default: return false;
        }
    }
    
    bool Types::isUnsigned(Type type)
    {
        switch(type)
        {
        case Type::u8:
        case Type::u16:
        case Type::u32:
        case Type::u64:
            return true;
        default: return false;
        }
    }
    
    bool Types::isFloating(Type type)
    {
        switch(type)
        {
        case Type::f32:
        case Type::f64:
            return true;
        default: return false;
        }
    }
}