# LINC Language: Linc Is Not C

*Disclaimer: Everything demonstrated here is a Work In Progress (W.I.P.), and the project is not yet ready for public use.*

Linc is a general-purpose programming language inspired by C.
Here are some of its key characteristics:

- **Statically typed:** Linc's static typing ensures that every value's purpose is respected, such that it be used accordingly.
- **Multi-paradigm:** It is a priority that the language support multiple coding-paradigms, in order to appeal to not only *Functional* programming audiences, but also *Object Oriented* and *Procedural* ones.
- **Free form:** Like most languages, indentation in Linc is insignificant. This means that one can indent their code according to their personal style. 
- ***Imperative:** At the current stage, imperative programming is emphasized, so as to guarantee that even the most fundamental of code is of high quality. In the future, declarative programming will also be approached. 

## Features

- **Immutability by default:** In the majority of modern programming languages, a variable is declared mutable unless stated otherwise (most commonly achieved using the 'const' keyword). In linc, as well as a few other functional-inspired programming languages (such as Rust), the opposite is true. This serves the purpose of easing-up const-correctness (it's much more difficult to specify const, when most of the time it makes sense as the default, rather 'opting-in' for mutability on a case-by-case basis).
- **Evaluator/Interpreter:** Although the language is far from 'complete' (which it most probably won't ever be, but will be kept in maintainance), one can already experiment with language features and the language in general, even before the assembly generator is implemented.

## Requirements

This project uses some well established tools to function. Those include:

- A system that supports OpenGL.
- CMake 3.16 (or newer version).
- A CMake generator (such as GNU Make or Ninja).
- Git (necessary so as to clone the repository, archive .zip downloads are highly discouraged).
- A terminal emulator that supports ANSI escape sequences.
- A c++ compiler with standard c++23 support, which is currently relatively limited.

The tools mentioned must be available in the $PATH environment variable, in order to be usable inside of the shell properly.

## Compiling

To start compiling this project, you must first clone the GitHub repository recursively.

```sh
git clone --recursive https://github.com/fosspointer/linc
```

### Linux and Unix-based systems

In order to ease the compilation process, the shell scripts *environment.sh* and *build.sh* have been provided. The first script, *environment.sh*, appends the build directory to the `$PATH` variable, so that a 'production ready' environment can be tested. The second script, *build.sh*, proceeds with the actual compilation process.

- Run *environment.sh* once after starting your terminal emulator of choice. You don't need to run it again for this terminal session.
- Use the provided build script (*build.sh*). Beware that it uses the Ninja generator by default, meaning that using a different generator (currently) requires executing the required commands manually.

### Windows

For Windows users, The use of WSL (Windows Subsystem for Linux) is highly recommended. Windows is tested to a lesser extent, which is due to the project being developed in a Linux development environment. For this reason, some builds using MSVC may not compile properly. Using a port of the GCC compiler such as MinGW may result in better luck in this regard.

## Miscellaneous

- See the [changelog](./changelog.md) for newest additions and next updates.
- For the curious reader, here's the [formal grammar](./grammar.md) of linc. 