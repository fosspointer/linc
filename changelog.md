# Changelog for linc version 0.6

- Changed comment tokenization from `#` to `//`.
- Changed preprocessor definition specifiers from `;` to `#`.
- The preprocessor now works on a token-basis, and executes after the lexer.
- Reimplemented the type system to make it more dynamic.
- Exposed the lexer and parser to preprocessor definition tokens, necessary to allow for the tokenization of the lexer.
- Introduced a standard library, which currently consists of `std.linc`, `stdmath.linc`, `stdstring.linc` and `stdio.linc`. 
- Functions now return void by default (in which case the return type can be omitted).
- Made all argument lists comma separated (arguments, arrays, etc...).
- Added right-associative binary operators (adjusted existing ones accordingly).
- Changed index expressions to be usable with all array expressions (not just identifiers).
- Added structures (`struct <identifier> { <vardecl>* }`).
- Added modulo operator (`%`).
- Added arithmetic assignment operators (`+=`, `-=`, `*=`, `/=`, `%=`).
- Added bitwise operators (`&`, `|`, `^`, `!!`, `<<`, `>>`).
- Added input from stdin (`readchar(), readraw(), readln()` internal function).
- Added external declarations (`ext <identifier> (<type_0>, <type_1>, ..., <type_n>): <type>`- helpful for future codegen and linking).
- Added support for nested arrays and their according types (e.g. `i32[3u64][2u64]`). This also adds nested array initializers.
- Added explicit and implicit structure initializers (e.g. the explicit assignment `position: vec2f32 = vec2f32{.x = 5f, .y = 3f}` can be reduced to `position: vec2f32{.x = 5f, .y = 3f}`).
- Renamed all instances of cloneConst() to clone() since it's a trivially const operation.
- Added return statements (`return <expression>`).
- Changed scopes/blocks to be expressions (such that blocks can be returned likewise- for example, `return {}` is now valid).
- Added label statements (e.g. `~foo`).
- Added jump statements (jump to a label, e.g. `jump foo`). 
- Added break and return statements (that optionally bind to labeled loops, similar to how it is done in Java).
- Heavily improved error handling, by adding file and line numbers to virtually all compile-time errors.
- ANSI Colors now work on a stack and are pushed/popped accordingly.
- Linc can now be installed and managed using the provided CMake installer (the standard headers are also installed).
- Added compilation and install scripts that ease the building process (assuming ninja/makefile present, ninja used by default).
- Added custom include directory suport (e.g `include <path/relative/to/include.linc>`). 
- Added command line arguments to the linc environment (-i to specify custom include directories and -e to evaluate a given expression).
- Made include-relative filepaths work on a token basis (which evaluate to their corresponding string literal).
- Added preprocessor macros (not to be confused with definitions; these take arguments and behave similar to 'function-like' macros in C/C++).
- Added binary, decimal and hexadecimal integral prefixes (`0d`, `0x`, `0b`).
- Removed the old `#instance` directives, since preprocessing happens after tokenization now.
- Removed `grammar.md`, as it is not useful and requires maintance.

## Future plans (in order of priority)

- Implement arrays of structures.
- References (non-nullable pointers to types, syntax still to be decided).
- Function pointers.
- Optional types.
- Dynamic arrays (details not yet decided).
- Improve error handling (line and syntax highlighting, less repetitive and more descriptive errors).