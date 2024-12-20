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
                Internal{"putln", Types::voidType, std::vector<Types::type>{Types::fromKind(Types::Kind::string)}}, 
                Internal{"putc", Types::voidType, std::vector<Types::type>{Types::fromKind(Types::Kind::_char)}}, 
                Internal{"readc", Types::fromKind(Types::Kind::_char), std::vector<Types::type>{}},
                Internal{"readraw", Types::fromKind(Types::Kind::string), std::vector<Types::type>{}},
                Internal{"readln", Types::fromKind(Types::Kind::string), std::vector<Types::type>{Types::fromKind(Types::Kind::string)}},
                Internal{"system", Types::fromKind(Types::Kind::string), std::vector<Types::type>{Types::fromKind(Types::Kind::string)}},
                Internal{"sys_read", Types::fromKind(Types::Kind::i64), std::vector<Types::type>{Types::fromKind(Types::Kind::i32), Types::fromKind(Types::Kind::string), Types::fromKind(Types::Kind::u64)}},
                Internal{"sys_write", Types::fromKind(Types::Kind::i64), std::vector<Types::type>{Types::fromKind(Types::Kind::i32), Types::fromKind(Types::Kind::string), Types::fromKind(Types::Kind::u64)}},
                Internal{"sys_open", Types::fromKind(Types::Kind::i32), std::vector<Types::type>{Types::fromKind(Types::Kind::string), Types::fromKind(Types::Kind::i32), Types::fromKind(Types::Kind::u16)}},
                Internal{"sys_close", Types::fromKind(Types::Kind::_bool), std::vector<Types::type>{Types::fromKind(Types::Kind::i32)}},
                Internal{"sys_exit", Types::voidType, std::vector<Types::type>{Types::fromKind(Types::Kind::i32)}}
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
