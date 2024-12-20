#pragma once
#include <linc/Binder.hpp>
#include <linc/generator/GeneratorAMD64.hpp>
#include <linc/generator/Target.hpp>

namespace linc
{
    class Generator final
    {
    public:
        Generator() = delete;
        static std::pair<std::string, bool> operator()(const BoundProgram* program, Target target)
        {
            switch(target.architecture)
            {
            case Target::Architecture::AMD64:
            {
                GeneratorAMD64 generator(program, target.platform);
                return generator.generateProgram();
            }
            case Target::Architecture::I386:
                throw LINC_EXCEPTION_INVALID_INPUT("i386 compatible compilation targets are not yet supported.");
            default: throw LINC_EXCEPTION_OUT_OF_BOUNDS(target.architecture);
            }
        }
    };
}