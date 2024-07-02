# LINC Language: Linc Is Not C

*Disclaimer: Everything demonstrated here is a Work In Progress (W.I.P.), and the project is not yet ready for public use. Having said that, it is functional and can be explored and tested.*

Linc is a general-purpose programming language inspired by C.
Here are some of its key characteristics:

- **Statically typed:** Linc's static typing ensures that every value's purpose is respected, such that it be used accordingly.
- **Multi-paradigm:** It is a priority that the language support multiple coding-paradigms, in order to appeal to not only *Functional* programming audiences, but also *Object Oriented* and *Procedural* ones.
- **Free form:** Like most languages, indentation in Linc is insignificant. This means that one can indent their code according to their personal style. 
- **Expressive:** A problem with many low-level languages is the lack of expressiveness. This is understandable, due to a focus on performance and compile-time efficiency, deeming them top priorities. In Linc, the language is designed to maximize expression, after which performance is taken into account in the implementation.

### Screenshot showcasing how the language feels.

For more, look into [examples](./examples/).

<img src="./assets/images/screenshot.png" style="max-width: 64rem" alt="Code example (generated using CodeSnap)."/>

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

You can then use the `install.sh` script to install Linc to your system.

### Linux and Unix-based systems

In order to ease the compilation process, the shell scripts *environment.sh* and *build.sh* have been provided. The first script, *environment.sh*, appends the build directory to the `$PATH` variable, so that a 'production ready' environment can be tested. The second script, *build.sh*, proceeds with the actual compilation process.

- Run *environment.sh* once after starting your terminal emulator of choice. You don't need to run it again for this terminal session.
- Use the provided build script (*build.sh*). Beware that it uses the Ninja generator by default, meaning that using a different generator (currently) requires executing the required commands manually.

### Windows

Changes have been made to support Microsoft Windows, for which the project is functional. Windows does receive specialized testing, though to a lesser extent than GNU/Linux. However, it is not the main environment used to develop and test the language, and any Windows specific problems might not receive fixes quickly. All windows testing has been done under MSYS2, MinGW-w64 (UCRT) and Ninja.

## Miscellaneous

- See the [changelog](./changelog.md) for newest additions and next updates.
- A [guide](./guide/0-getting_started.md) is provided for those interested in learning the Linc language. 