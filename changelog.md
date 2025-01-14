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

## Future plans (in order of priority)

- References (non-nullable pointers to types, syntax still to be decided).
- Dynamic arrays (details not yet decided).
- Improve error handling (line and syntax highlighting, less repetitive and more descriptive errors) (half done).