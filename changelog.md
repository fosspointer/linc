# Changelog for linc version 0.4

- Implemented documentation support using Doxygen (along with cmake target).
- Introduced precompiled header for common STL includes.
- Refactored and cleaned up the binder, parser and bound-syntax-tree nodes.
- The mutability keyword must now precede variable declarations (e.g. `mut i32 foo` instead of `i32 foo mut`).
- Added assets directory (linc's official logo).
- Introduced declarations as a new AST/ABT node type.
- Added preprocessor directive support.
- Added preprocessor include directive support (copy-paste the contents of a file).
- Added support for preprocessor word and instance definitions.
- Implemented function declarations (`fn <function_name> ((mut) <typename> <varname> ...): <typename> <statement>`).
- Added function call expressions (`<function_name>(<expression> ...)`).
- Renamed the linc environment source/binary to 'lincenv' (previously 'lincc').
- Moved the formed 'Evaluator' class to the generator directory and renamed it to 'Interpreter'.
- Added virtual function clone_const() to all ABT nodes (similar to function of the same name in AST nodes).
- All programs are now equivalent to lists of declarations, and not single statements.
- Added entry point function main(), which is required for a program to be evaluated (is currently argumentless, returns void/i8-i32/u8).
- When the entry-point function returns, the return value is now used as the process's exit code.

## Ideas for version 0.5

- Improve testing, as it is currently only manual. 
- Type casting (convertions between types).
- Add 'type' type and typeof (':') operator.
- Add right-associative binary operators (along with the pre-existing left-associative ones). 
- Implement static arrays.
- String indexing.
- For loops. 

## Future plans

- An implementation of optional types (details not yet decided).
- References (non-nullable pointers to types, syntax still to be decided).
- Dynamic arrays (details not yet decided).
- Optional types.
- Function pointers.
- Switch execution flow to start at the reserved (and required) main function as the entry point (optional string-array argument) (after implementation of functions).
- Improve error handling (line and syntax highlighting, less repetitive and more descriptive errors).