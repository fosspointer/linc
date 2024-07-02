#pragma once
#include <linc/system/Exception.hpp>
#include <linc/Include.hpp>

#define LINC_TYPES_KIND_TYPE unsigned char

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

        enum Size: LINC_TYPES_KIND_TYPE
        {
            Zero = 0, Byte = 1, Word = 2, DoubleWord = 4, QuadWord = 8
        };

        class type final
        {
        public:
            enum class Kind
            {
                Primitive, Array, Structure
            };

            using Primitive = Types::Kind;
            using Structure = std::vector<std::pair<std::string, std::unique_ptr<const type>>>;
            struct Array final
            {
                std::unique_ptr<const type> base_type;
                std::optional<std::size_t> count;
            };

            type(Primitive primitive, bool is_mutable = false)
                :kind(Kind::Primitive), primitive(primitive), isMutable(is_mutable)
            {}

            type(Structure _structure, bool is_mutable = false)
                :kind(Kind::Structure), isMutable(is_mutable)
            {
                new (&structure) std::vector{cloneStructure(&_structure, is_mutable)};
            }

            type(const Array& _array, bool is_mutable = false)
                :kind(Kind::Array), isMutable(is_mutable)
            {
                new (&array) Array{.base_type = _array.base_type? _array.base_type->clone(): nullptr, .count = _array.count};
            }

            ~type()
            {
                if(kind == Kind::Array)
                    array.~Array();
                else if(kind == Kind::Structure)
                    structure.~vector();
            }

            type(const type& other)
                :isMutable(other.isMutable), kind(other.kind)
            {
                switch(kind)
                {
                case Kind::Primitive: primitive = other.primitive; break;
                case Kind::Array: new (&array) Array{.base_type = other.array.base_type->clone(), .count = other.array.count}; break;
                case Kind::Structure: new (&structure) std::vector{cloneStructure(&other.structure, other.isMutable)}; break;
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::type::Kind);
                }
            }

            type(type&& other)
                :isMutable(other.isMutable), kind(other.kind)
            {
                switch(kind)
                {
                case Kind::Primitive: primitive = other.primitive; break;
                case Kind::Array: new (&array) Array{std::move(other.array)}; break;
                case Kind::Structure: new (&structure) std::vector{std::move(other.structure)}; break;
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::type::Kind);
                }
            }

            type& operator=(const type& other)
            {
                kind = other.kind;
                isMutable = other.isMutable;

                switch(kind)
                {
                case Kind::Primitive: primitive = other.primitive; break;
                case Kind::Array: new (&array) Array{.base_type = other.array.base_type->clone(), .count = other.array.count}; break;
                case Kind::Structure: new (&structure) std::vector{cloneStructure(&other.structure, other.isMutable)}; break;
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::type::Kind);
                }
                return *this;
            }

            type& operator=(type&& other)
            {
                kind = other.kind;
                isMutable = other.isMutable;

                switch(kind)
                {
                case Kind::Primitive: primitive = other.primitive; break;
                case Kind::Array: new (&array) Array{std::move(other.array)}; break;
                case Kind::Structure: new (&structure) std::vector{std::move(other.structure)}; break;
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::type::Kind);
                }
                return *this;
            }

            std::unique_ptr<const type> clone() const
            {
                switch(kind)
                {
                case Kind::Primitive: return std::make_unique<const type>(primitive, isMutable);
                case Kind::Array: return std::make_unique<const type>(Array{.base_type = array.base_type->clone(), .count = array.count}, isMutable);
                case Kind::Structure:
                {
                    Structure structure_vector;
                    for(const auto& member: structure)
                        structure_vector.push_back(std::pair(member.first, member.second->clone()));
                 
                    return std::make_unique<const type>(std::move(structure_vector), isMutable);
                }
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::type::Kind);
                }
            }

            bool operator==(const type& _other) const
            {
                if(isMutable != _other.isMutable || kind != _other.kind)
                    return false;
                
                switch(kind)
                {
                case Kind::Primitive: return primitive == _other.primitive;
                case Kind::Array: return *array.base_type == *_other.array.base_type && array.count == _other.array.count;
                case Kind::Structure: 
                    {
                        if(structure.size() != _other.structure.size()) return false;

                        for(Structure::size_type i{0ul}; i < structure.size(); ++i)
                            if(*structure[i].second != *_other.structure[i].second)
                                return false;

                        return true;
                    }
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::type::Kind);
                }
            }

            bool operator!=(const type& other) const 
            {
                return !(*this == other);
            }
            
            bool isAssignableTo(const type& other) const
            {
                if(kind != other.kind) return false;

                switch(kind)
                {
                case Kind::Primitive: return primitive == other.primitive;
                case Kind::Array:
                    if(array.count && *array.count == 0ul) return true;
                    if(!array.base_type->isAssignableTo(*other.array.base_type))
                        return false;
                    else return (array.count && other.array.count && *array.count == *other.array.count) || !other.array.count;
                case Kind::Structure:
                    if(structure.size() != other.structure.size()) return false;

                    for(Structure::size_type i{0ul}; i < structure.size(); ++i)
                        if(!structure.at(i).second || !other.structure.at(i).second) return false;
                        else if(!structure[i].second->isAssignableTo(*other.structure[i].second)) return false;
                
                    return true;
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(Types::type::Kind);
                }
            }

            bool isCompatible(const type& other) const
            {
                return isAssignableTo(other) || other.isAssignableTo(*this);
            }

            bool isMutable;
            union
            {
                Primitive primitive;
                Array array;
                Structure structure;
            };
            Kind kind;

            static Structure cloneStructure(const Structure* structure, bool is_mutable)
            {
                Structure result;

                for(const auto& type: *structure)
                {
                    auto clone = type.second->clone();
                    const_cast<Types::type*>(clone.get())->isMutable = clone->isMutable || is_mutable;
                    result.push_back(std::pair(type.first, std::move(clone)));
                }

                return std::move(result);
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
        struct _void_type { inline bool operator==(const _void_type&) const { return true; } };
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
        static std::string toString(const type& type, bool ignore_mutability = false);
        static type fromKind(Kind kind);
        static Kind kindFromString(const std::string& value);
        static Kind kindFromUserString(const std::string& value);
        static Kind kindFromUserStringSuffix(const std::string& value);
        static Size sizeFromKind(Kind kind);
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