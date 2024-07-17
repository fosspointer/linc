#pragma once
#include <linc/Include.hpp>
#include <linc/system/Types.hpp>
#include <linc/system/Files.hpp>

namespace linc
{
    class Internals final
    {
    public:
        struct Internal
        {
            std::string name;
            Types::type returnType;
            std::vector<Types::type> arguments;
        };

        static auto& get()
        {
            static std::vector<Internal> internals{
                Internal{"puts", Types::voidType, std::vector<Types::type>{Types::fromKind(Types::Kind::string)}}, 
                Internal{"putc", Types::voidType, std::vector<Types::type>{Types::fromKind(Types::Kind::_char)}}, 
                Internal{"i32_length", Types::fromKind(Types::Kind::u64), std::vector<Types::type>{Types::fromKind(Types::Kind::i32)}},
                Internal{"i32_to_string", Types::fromKind(Types::Kind::string), std::vector<Types::type>{Types::fromKind(Types::Kind::i32)}},
                Internal{"u64_to_string", Types::fromKind(Types::Kind::string), std::vector<Types::type>{Types::fromKind(Types::Kind::u64)}},
                Internal{"__puts_literal", Types::voidType, std::vector<Types::type>{Types::fromKind(Types::Kind::string), Types::fromKind(Types::Kind::u64)}},
                Internal{"memory_alloc", Types::fromKind(Types::Kind::u64), std::vector<Types::type>{Types::fromKind(Types::Kind::u64)}},
                Internal{"readraw", Types::fromKind(Types::Kind::string), std::vector<Types::type>{}},
                Internal{"readchar", Types::fromKind(Types::Kind::_char), std::vector<Types::type>{}},
                Internal{"readln", Types::fromKind(Types::Kind::string), std::vector<Types::type>{Types::fromKind(Types::Kind::string)}},
                Internal{"system", Types::fromKind(Types::Kind::string), std::vector<Types::type>{Types::fromKind(Types::Kind::string)}},
                Internal{"exit", Types::voidType, std::vector<Types::type>{Types::fromKind(Types::Kind::i32)}}
            };
            return internals;
        }

        static bool isInternal(std::string_view symbol_name)
        {
            for(const auto& declaration: get())
                if(declaration.name == symbol_name)
                    return true;
            
            return false;
        }

        static std::vector<Internal>::iterator find(std::string_view symbol_name)
        {
            auto& internals = get();

            for(auto it = internals.begin(); it != internals.end(); ++it)
                if(it->name == symbol_name)
                    return it;

            return internals.end();
        }
    };
}
