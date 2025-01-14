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
                Primitive, Array, Structure, Function, Enumeration
            };

            using Primitive = Types::Kind;
            using Structure = std::vector<std::pair<type, std::string>>;
            using Enumeration = std::vector<std::pair<std::string, type>>;

            struct Array final
            {
                std::unique_ptr<const type> baseType;
                std::optional<std::size_t> count;
            };

            struct Function final
            {
                std::unique_ptr<const type> returnType;
                std::vector<type> argumentTypes;
            };

            type(Primitive primitive, bool is_mutable = false)
                :kind(Kind::Primitive), primitive(primitive), isMutable(is_mutable)
            {}

            type(const Structure& _structure, bool is_mutable = false)
                :kind(Kind::Structure), isMutable(is_mutable)
            {
                new (&structure) Structure{_structure};
            }

            type(const Enumeration& _enumeration, bool is_mutable = false)
                :kind(Kind::Enumeration), isMutable(is_mutable)
            {
                new (&enumeration) std::vector{_enumeration};
            }

            type(const Array& _array, bool is_mutable = false)
                :kind(Kind::Array), isMutable(is_mutable)
            {
                new (&array) Array{.baseType = _array.baseType? _array.baseType->clone(): nullptr, .count = _array.count};
            }

            type(const Function& _function, bool is_mutable = false)
                :kind(Kind::Function), isMutable(is_mutable)
            {
                new (&function) Function{cloneFunction(&_function)};
            }

            ~type()
            {
                switch(kind)
                {
                case Kind::Array: array.~Array(); break;
                case Kind::Structure: structure.~vector(); break;
                case Kind::Function: function.~Function(); break;
                case Kind::Enumeration: enumeration.~vector(); break;
                default: break;
                }
            }

            type(const type& other)
                :kind(other.kind), isMutable(other.isMutable)
            {
                switch(kind)
                {
                case Kind::Primitive: primitive = other.primitive; break;
                case Kind::Array: new (&array) Array{.baseType = other.array.baseType->clone(), .count = other.array.count}; break;
                case Kind::Structure: new (&structure) Structure{cloneStructure(other.structure, isMutable)}; break;
                case Kind::Function: new (&function) Function{cloneFunction(&other.function)}; break;
                case Kind::Enumeration: new (&enumeration) Enumeration{other.enumeration}; break;
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(kind);
                }
            }

            type(type&& other)
                :kind(other.kind), isMutable(other.isMutable)
            {
                switch(kind)
                {
                case Kind::Primitive: primitive = other.primitive; break;
                case Kind::Array: new (&array) Array{std::move(other.array)}; break;
                case Kind::Structure: new (&structure) std::vector{std::move(other.structure)}; break;
                case Kind::Function: new (&function) Function{std::move(other.function)}; break;
                case Kind::Enumeration: new (&enumeration) Enumeration{std::move(other.enumeration)}; break;
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(kind);
                }
            }

            type& operator=(const type& other)
            {
                kind = other.kind;
                isMutable = other.isMutable;

                switch(kind)
                {
                case Kind::Primitive: primitive = other.primitive; break;
                case Kind::Array: new (&array) Array{.baseType = other.array.baseType->clone(), .count = other.array.count}; break;
                case Kind::Structure: new (&structure) std::vector{cloneStructure(other.structure, isMutable)}; break;
                case Kind::Function: new (&function) Function{cloneFunction(&other.function)}; break;
                case Kind::Enumeration: new (&enumeration) Enumeration{other.enumeration}; break;
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(kind);
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
                case Kind::Function: new (&function) Function{std::move(other.function)}; break;
                case Kind::Enumeration: new (&enumeration) Enumeration{std::move(other.enumeration)}; break;
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(kind);
                }
                return *this;
            }

            std::string toString(bool ignore_mutability = false) const;

            std::unique_ptr<const type> clone() const
            {
                switch(kind)
                {
                case Kind::Primitive: return std::make_unique<const type>(primitive, isMutable);
                case Kind::Array: return std::make_unique<const type>(Array{.baseType = array.baseType->clone(), .count = array.count}, isMutable);
                case Kind::Structure:
                {
                    Structure structure_vector;
                    for(const auto& member: structure)
                        structure_vector.push_back(std::pair(member.first, member.second));
                 
                    return std::make_unique<const type>(std::move(structure_vector), isMutable);
                }
                case Kind::Function:
                {
                    return std::make_unique<const type>(Function{.returnType = function.returnType->clone(), .argumentTypes = function.argumentTypes},
                        isMutable);
                }
                case Kind::Enumeration: return std::make_unique<const type>(enumeration, isMutable);
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(kind);
                }
            }

            bool operator==(const type& other) const
            {
                if(isMutable != other.isMutable || kind != other.kind)
                    return false;
                
                switch(kind)
                {
                case Kind::Primitive: return primitive == other.primitive;
                case Kind::Array: return *array.baseType == *other.array.baseType && array.count == other.array.count;
                case Kind::Structure: 
                {
                    if(structure.size() != other.structure.size()) return false;

                    for(Structure::size_type i{0ul}; i < structure.size(); ++i)
                        if(structure[i].first != other.structure[i].first)
                            return false;

                    return true;
                }
                case Kind::Function:
                {
                    if(!function.returnType || !other.function.returnType || *function.returnType != *other.function.returnType
                    || function.argumentTypes.size() != other.function.argumentTypes.size()) return false;

                    for(decltype(Function::argumentTypes)::size_type i{0ul}; i < function.argumentTypes.size(); ++i)
                        if(function.argumentTypes[i] != other.function.argumentTypes[i])
                            return false;

                    return true;
                }
                case Kind::Enumeration: return enumeration == other.enumeration;
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(kind);
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
                    if(!array.baseType->isAssignableTo(*other.array.baseType))
                        return false;
                    else return (array.count && other.array.count && *array.count == *other.array.count) || !other.array.count;
                case Kind::Structure:
                    if(structure.size() != other.structure.size()) return false;

                    for(Structure::size_type i{0ul}; i < structure.size(); ++i)
                        if(!structure[i].first.isAssignableTo(other.structure[i].first)) return false;
                
                    return true;
                case Kind::Function: return type(function) == type(other.function);
                case Kind::Enumeration: return enumeration == other.enumeration;
                default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(kind);
                }
            }

            bool isCompatible(const type& other) const
            {
                return isAssignableTo(other) || other.isAssignableTo(*this);
            }

            Kind kind;
            union
            {
                Primitive primitive;
                Array array;
                Structure structure;
                Function function;
                Enumeration enumeration;
            };
            bool isMutable;

            static Structure cloneStructure(Structure structure, bool is_mutable)
            {
                for(auto& type: structure)
                    type.first.isMutable |= type.first.isMutable;
                
                return structure;
            }

            static Function cloneFunction(const Function* function)
            {
                return Function{.returnType = function->returnType->clone(), .argumentTypes = function->argumentTypes};
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
        [[nodiscard]] inline static const std::unique_ptr<To> uniqueCastDynamic(std::unique_ptr<From> p)
        {
            std::unique_ptr<To> result(dynamic_cast<To*>(p.get()));
            
            if(result)
            {
                p.release();
                return result;
            }
            return nullptr;
        }

        template <typename To, typename From> 
        [[nodiscard]] inline static const std::unique_ptr<To> uniqueCast(std::unique_ptr<From> p)
        {
            std::unique_ptr<To> result(static_cast<To*>(p.get()));
            p.release();
            return std::move(result);
        }

        static Variant toVariant(Kind type, const std::string& value);
        static std::string kindToString(Kind kind);
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