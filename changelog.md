# Changelog for linc version 0.7.1
- Language: Improved for loop syntax: `for <clause> <expression>`, where `<clause>` is one of `<declaration>;<expression>;<expression>` (legacy for loop) `<identifier> in <expression>` (iterator based for loop).
- Language: Added range expressions <expression>..<expression> (for expressions that have intercompatible types and are incrementable).
- Language: Added the reverse (`-`) operator for ranges (e.g. `-(2..7) is the reverse range of (2..7)`). 
- Interpreter: Expanded arrays so as to support all kinds of values (including structures and enumerators, as opposed to just primitives and other arrays).
- Language: Cleared up some of the operator validity rules for additions and assignments.
- REPL: Fixed SIGINT (Ctrl+C) handling and made it applicable only when reading input from lincenv.
- Interpreter: Fixed arrays leaking memory when moving values (1/2).
- Misc: Added the build year as a preprocessor definition, used for the legal notice.
- Misc: Improved minor details (such as fixing the use of struct for class forward declarations and vice-versa, removing std::move on clone, etc...).
- Misc: Made compilation with clang possible.
- Misc: Reverted the change where __FILE__ and __LINE__ were redefined for release builds. 
- Language: Generalized functions via function pointers, which can be passed to functions and variables as expressions.
- Language: Introduced generic declarations with the following syntax: `gen<<identifer>*> <declaration>`, where `<identifier>*` is a delimited list of identifier types, to be specified by the instances, and `<declaration>` is an (external)function, enumeration or structure declaration. Example: `gen<T, U> fn add(first: T, second: U) first + as T(second)` defines a generic function on a pair of types T,U that converts the second argument to be of the same type as the first, then adds both arguments together. This function works for any pair of numeric types.
- Language: Identifiers now support generic clauses, enabling for generic monomorphization to take place. Example: this is the code to call `add<T, U>` for types `f64` and `u16` (values 1.618034 and 256): `add<f64, u16>(1.618034f64, 256u16)`.
- Language: Support for type aliases (equivalent to C's typedefs) is now available with the following syntax `alias <identifier> = <type>`.
- Language: Cleaned up the syntax for structure initializers (something like `Vec2<f32>{.x = 5, .y = 3}` now becomes `Vec2<f32>{x = 5, y = 3}`).
- REPL: Added support for inline preprocessor definitions (`-D`, `--define <name>=<value>`) as well as an option to disable ANSI (`-a`, `--disable-ansi`).

## Future plans (in order of priority)

- More standard library tooling.
- References (non-nullable pointers to types, syntax still to be decided).
- Distinction between static and dynamic arrays.
- Module system?