#include <linc/system/Types.hpp>

#define LINC_TYPE_MAP_PAIR(first, second) std::pair<std::string, linc::Types::Kind>(first, linc::Types::Kind::second)

namespace linc
{
    Types::type Types::voidType = Types::fromKind(Kind::_void);
    Types::type Types::invalidType = Types::fromKind(Kind::invalid);

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
        LINC_TYPE_MAP_PAIR("void", _void),
        LINC_TYPE_MAP_PAIR("bool", _bool),
        LINC_TYPE_MAP_PAIR("string", string),
        LINC_TYPE_MAP_PAIR("type", type)
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
        LINC_TYPE_MAP_PAIR("d", f64),

        LINC_TYPE_MAP_PAIR("c", _char),
        LINC_TYPE_MAP_PAIR("b", _bool)
    };

    Types::Variant Types::toVariant(Kind kind, const std::string& value)
    {
        switch(kind)
        {
        case Kind::u8: return static_cast<u8>(std::stoul(value)); 
        case Kind::u16: return static_cast<u16>(std::stoul(value));
        case Kind::u32: return static_cast<u32>(std::stoul(value));
        case Kind::u64: return static_cast<u64>(std::stoull(value));
        case Kind::i8: return static_cast<i8>(std::stoi(value)); 
        case Kind::i16: return static_cast<i16>(std::stoi(value));
        case Kind::i32: return static_cast<i32>(std::stoi(value));
        case Kind::i64: return static_cast<i64>(std::stoull(value));
        case Kind::f32: return static_cast<f32>(std::stof(value));
        case Kind::f64: return static_cast<f64>(std::stod(value));
        case Kind::_char: return static_cast<_char>(value.at(0));
        case Kind::_bool: return parseBoolean(value);
        case Kind::string: return value.c_str();
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(kind);
        }
    }

    std::string Types::kindToString(Kind kind)
    {
        switch(kind)
        {
        case Kind::invalid: return "<invalid_type>";
        case Kind::u8: return "u8";
        case Kind::u16: return "u16";
        case Kind::u32: return "u32";
        case Kind::u64: return "u64";
        case Kind::i8: return "i8";
        case Kind::i16: return "i16";
        case Kind::i32: return "i32";
        case Kind::i64: return "i64";
        case Kind::f32: return "f32";
        case Kind::f64: return "f64";
        case Kind::_char: return "char";
        case Kind::_bool: return "bool";
        case Kind::string: return "string";
        case Kind::_void: return "void";
        case Kind::type: return "type";
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(kind);
        }
    }

    std::string Types::type::toString(bool ignore_mutability) const
    {
        std::string result;
        if(isMutable && !ignore_mutability && kind != type::Kind::Function)
            result.append("mut ");

        switch(kind)
        {
        case type::Kind::Primitive: result.append(kindToString(primitive)); break;
        case type::Kind::Array:
            result.append(array.baseType->toString(true));
            result.append(array.count? "[" + std::to_string(*array.count) + "]": "[]");
            break;
        case type::Kind::Structure:
            result.push_back('{');
            for(type::Structure::size_type i{0ul}; i < structure.size(); ++i)
                result.append((i == 0ul? "": ", ") + structure[i].first + ": " + structure[i].second->toString());
            result.push_back('}');
            break;
        case type::Kind::Function:
            result.append("fn(");
            for(decltype(type::Function::argumentTypes)::size_type i{0ul}; i < function.argumentTypes.size(); ++i)
                result.append((i == 0ul? "": ", ") + function.argumentTypes[i]->toString());
            result.append("): " + function.returnType->toString());
            break;
        case type::Kind::Enumeration:
            result.append("enum(");
            for(type::Enumeration::size_type i{0ul}; i < enumeration.size(); ++i)
                result.append((i == 0ul? "": ", ") + enumeration[i].first);
            result.push_back(')');
            break;
        default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(kind);
        }   

        return result;
    }

    auto Types::fromKind(Kind kind) -> type
    {
        return type(kind);
    }

    Types::Kind Types::kindFromString(const std::string& value)
    {
        auto find = s_typeMap.find(value);
        
        if(find != s_typeMap.end())
            return find->second;
        else throw LINC_EXCEPTION("String does not correspond to a type");
    }

    Types::Kind Types::kindFromUserString(const std::string& value)
    {
        auto f = s_typeMap.find(value);
        
        if(f != s_typeMap.end())
            return f->second;
        else return Kind::invalid;
    }

    Types::Kind Types::kindFromUserStringSuffix(const std::string& value)
    {
        auto f = s_suffixMap.find(value);
        
        if(f != s_suffixMap.end())
            return f->second;
        else return Kind::invalid;
    }

    Types::Size Types::sizeFromKind(Kind kind)
    {
        switch(kind)
        {
        case Kind::_bool:
        case Kind::u8:
        case Kind::i8:
        case Kind::_char:
            return Size::Byte;
        case Kind::i16:
        case Kind::u16:
            return Size::Word;
        case Kind::i32:
        case Kind::u32:
        case Kind::f32:
            return Size::DoubleWord;
        case Kind::f64:
        case Kind::u64:
        case Kind::i64:
        case Kind::string:
            return Size::QuadWord;
        case Kind::_void:
        case Kind::invalid:
        default:
            return Size::Zero;
        }
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

    Types::_char Types::parseUserCharacter(const std::string& str)
    {
        char* p;
        char character = static_cast<char>(strtoll(str.c_str(), &p, 10));
            
        if(*p)
        {
            if(str.empty() || str.size() > 1ul)
                return '\0';
            else return str[0];
        }
        return character;
    }

    bool Types::isNumeric(Kind type)
    {
        switch(type)
        {
        case Kind::i8:
        case Kind::i16:
        case Kind::i32:
        case Kind::i64:
        case Kind::u8:
        case Kind::u16:
        case Kind::u32:
        case Kind::u64:
        case Kind::f32:
        case Kind::f64:
            return true;
        default: return false;
        }
    }

    bool Types::isIntegral(Kind type)
    {
        switch(type)
        {
        case Kind::i8:
        case Kind::i16:
        case Kind::i32:
        case Kind::i64:
        case Kind::u8:
        case Kind::u16:
        case Kind::u32:
        case Kind::u64:
            return true;
        default: return false;
        }
    }
    
    bool Types::isSigned(Kind type)
    {
        switch(type)
        {
        case Kind::i8:
        case Kind::i16:
        case Kind::i32:
        case Kind::i64:
            return true;
        default: return false;
        }
    }
    
    bool Types::isUnsigned(Kind type)
    {
        switch(type)
        {
        case Kind::u8:
        case Kind::u16:
        case Kind::u32:
        case Kind::u64:
            return true;
        default: return false;
        }
    }
    
    bool Types::isFloating(Kind type)
    {
        switch(type)
        {
        case Kind::f32:
        case Kind::f64:
            return true;
        default: return false;
        }
    }
}