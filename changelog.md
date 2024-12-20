# Changelog for linc version 0.7

- Codegen: Added Linux AMD64 experimental partial codegen support.
- Language: Changed hexadecimal literals to only allow for uppercase letters (so as to potentially allow floating point literals in the future; do keep in mind that `f32` and `f64` both contain the character `f`, which would have otherwise been interpreted as a digit).
- Misc: Minor changes.
- Codegen: Changed the 'stdinternal.s' assembly filename to 'lincinternal.s'.
- Codegen: Added more internal utilies (e.g. stringification, syscalls). 
- Codegen: Emitter improvements (plus identifier additions).
- Codegen: Added return statements.
- Codegen: Implemented type expressions (plus type-of operations).
- Codegen: Hugely improved stringification operations.
- Codegen: Fixed literal expression signedness.
- Codegen: Fixed block expression stack adjustion (for scoping).
- Assets: Updated linc icon.
- Language: Linc internal shared library (building + linking).
- Codegen: Added scoped symbols (fixing differently scoped 'name collisions', which shouldn't exist).
- Codegen: Added basic statically-stored variables (public variables).
- Language: Functions can now implicitly determine their return type (emitting it does not assume void).
- Codegen: while expressions now support else and finally clauses.
- Codegen: added and improved string operations (plus string indexing). 
- Language: Added terminator `;` token for all statements.
- Language: Made bodies of if/while expressions, as well as those of function declarations statements. 
- Language: Made expression statements have void type.
- Language: Added tail-expressions to blocks.
- Misc: error reporting improvements.
- Language: Made array typed variables with no default value and no explicit size invalid (instead of defaulting to 0).
- Misc: Refactoring and adjusting codegen for previous changes.
- Language: Added function pointer types (no function pointers for now).
- Misc: Cleaned up the type system (removed typing from statements).
- Misc: Switched from `std::optional<std::unique_ptr>` to just `std::unique_ptr` (`std::nullopt` -> `nullptr`).
- Misc: Added dependency checks for `nasm` and `ld` (for lincc).
- Misc: Made object and assembly output take place in the same directory as the resulting binary.
- Misc: Reporting is now simpler: `[$]` for categorization was changed to `$`.
- Language: Changed range-based for loops to make the order more natural (the new syntax is `for(<identifier> in <array_identifier>)`). 
- Interpreter: Fixed control flow issues regarding return, break and continue statements by switching to an exception(non-erroneous) based approach.
- Language: Changed labels to only apply to loops.
- Language: Removed jump statements.
- Misc: Fixed nested symbols with the same name overwriting each other.
- Language: Added variable shadowing.
- Misc: Improved error reporting once again (plus switched to using literal representations for symbols).
- Misc: Cleaned up examples directory, which now has code from only the latest iteration of the language.
- Misc: Slightly improved exception handling (for bugs during language analysis), making it more imformative and easier to debug.
- Misc: Greately simplified structures requiring sequences of delimited nodes, utilizing the new Clause structure.
- Language: Added enumerations and enumerator expressions.
- Language: Added a basic implementation of a match expression.
- Language: Made the return statement argument optional (returning void when omitted).
- Language: Enumerators now support optional values, functioning as sum types.
- Language: Introduced identifier pattern matching for enumerators.
- Compatibility: Fixed compatibility on Windows (disabling the codegen part of the project for Windows builds— assuming gcc 14 or greater).
- Guide: Updated guide to conform to linc version 0.6.1.
- Guide: Fixed minor mistakes.

(Note that all codegen related features are experimental)

## Future plans (in order of priority)

- Implement arrays of any values (as opposed to just primitives and nested arrays).
- References (non-nullable pointers to types, syntax still to be decided).
- Function pointers (half done).
- Dynamic arrays (details not yet decided).
- Improve error handling (line and syntax highlighting, less repetitive and more descriptive errors) (half done).