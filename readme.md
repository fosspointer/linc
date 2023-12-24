# LINC Language: Linc Is Not C

*Disclaimer: Everything demonstrated here is a Work In Progress (W.I.P.), and the project is not yet ready for public use.*

Linc is a general-purpose programming language inspired by C.
Here are some of its key characteristics:

- **Statically typed:** Linc's static typing ensures that every value's purpose is respected, such that it be used accordingly.
- **Multi-paradigm:** It is a priority that the language support multiple coding-paradigms, in order to appeal to both *Functional* programming audiences, as well as *Object Oriented* and *Procedural* ones.
- **Free form:** Like most languages, indentation in Linc is insignificant. This means that one can indent their code according to their personal style. 
- ***Imperative:** At the current stage, imperative programming is emphasized, so as to guarantee that even the most fundamental of code is of high quality. In the future, declarative programming will also be approached. 

## Requirements

This project uses some well established tools to function. Those include:

- A system that supports OpenGL.
- CMake 3.16 (or newer version).
- A CMake generator (such as GNU Make or Ninja).
- Git (necessary so as to clone the repository, archive .zip downloads are highly discouraged).
- A terminal emulator that supports ANSI escape sequences.
- A c++ compiler with standard c++23 support, which is currently relatively limited. For example, as of writing this, the header <stdfloat> is provided solely on GCC.

The tools mentioned must be available in the $PATH environment variable, in order to be usable inside of the shell properly.

## Compiling

To start compiling this project, you must first clone the GitHub repository recursively.

```sh
git clone --recursive https://github.com/fosspointer/
```

### Linux and Unix-based systems

In order to ease the compilation process, the shell scripts *environment.sh* and *build.sh* have been provided. The first script, *environment.sh*, appends the build directory to the `$PATH` variable, so that a 'production ready' environment can be tested. The second script, *build.sh*, proceeds with the actual compilation process.

- Run *environment.sh* once after starting your terminal emulator of choice. You don't need to run it again for this terminal session.
- Use the provided build script (*build.sh*). Beware that it uses the Ninja generator by default, meaning that using a different generator (currently) requires executing the required commands manually.

### Windows

For Windows users, The use of WSL (Windows Subsystem for Linux) is highly recommended. Windows is tested to a lesser extent, which is due to the project being developed in a Linux development environment. For this reason, some builds using MSVC may not compile properly. Using a port of the GCC compiler such as MinGW may result in better luck in this regard.

### Changelog for version 0.2

- Created basic language specification (only grammar is provided for now).
- Added the concept of statement return types.
- New type 'void', mainly used by statements.
- Added if/else expressions.
- Added while loop expressions.
- Added `putc` and `puts` statements (for printing characters and strings to stdout).
- Removed 'elif' keyword.
- Removed unnecessary field `isValid` from `NodeInfo`, as its intended functionality is well-covered by the reporting class for now. 
- Improved const-correctness by switching any immutable fields to `const`.
- Added new command, `/vars` (display a listof all declared variables).
- Added inline file evaluation support (when argc >= 2).

### Next additions

- Improve string handling capabilities.
- Add 'to string' operator (`@`) for all primitives (including `string`).
- Implement scoping (all variables are currently 'globally-scoped').
- Refactor `TypedValue` to use `std::variant`, so as to improve handling of strings.
- Change while loops to have 2 additional optional parts: `finally`, `else` (in the order provided).
- Add right-associativity for binary operators. 
- Add variable assignment expressions for mutable (using `var`) variables.
- Improve testing, as it is currently only manual. 
- More coming soon.