# Changelog for linc version 0.7.1
- Language: Improved for loop syntax: `for <clause> <expression>`, where `<clause>` is one of `<declaration>;<expression>;<expression>` (legacy for loop) `<identifier> in <expression>` (iterator based for loop).
- Language: Added range expressions <expression>..<expression> (for expressions that have intercompatible types and are incrementable).
- Language: Added the reverse (`-`) operator for ranges (e.g. `-(2..7) is the reverse range of (2..7)`). 

## Future plans (in order of priority)

- Implement arrays of any values (as opposed to just primitives and nested arrays).
- References (non-nullable pointers to types, syntax still to be decided).
- Function pointers (half done).
- Dynamic arrays (details not yet decided).
- Improve error handling (line and syntax highlighting, less repetitive and more descriptive errors) (half done).