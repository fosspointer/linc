# Changelog for linc version 0.5

- Introduced automated testing using CTest.
- Removed argument declarations- variable declarations are now used.
- Fixed integer-literal overflowing inconsistencies. 
- Type casting (convertions between types).
- Basic escape characters and fixed existing ones.
- Type expressions in the language (used when initializing variables).
- Added compile-time type-reflection.
- For loops (range and variable for-loops).
- Implemented arrays and array initializers (e.g. `array: i32[] = [1 2 3]`).
- Added array-indexing (for arrays and strings).
- Added type-of operator (':').
- Made default mutable values implicit. 
- Function default values are now used when appropriate.
- Basis for error-handling improvements.
- Added "else" requirement for typed while loops, which are now typed void by default.
- Changed variable declaration syntax to `<variable>: <type> = <value>` (the assignment is optional).

## Future plans

- Add right-associative binary operators (along with the pre-existing left-associative ones). 
- An implementation of optional types (details not yet decided).
- References (non-nullable pointers to types, syntax still to be decided).
- Dynamic arrays (details not yet decided).
- Optional types.
- Function pointers.
- Add arguments to the entry-point function.
- Improve error handling (line and syntax highlighting, less repetitive and more descriptive errors).