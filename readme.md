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

The tools mentioned must be available in the $PATH environment variable, in order to be usable inside of the shell properly.

## Compiling

To start compiling this project, you must first clone the GitHub repository recursively.

```sh
git clone --recursive https://github.com/fosspointer/
```

### Linux

In order to ease the compilation process, the shell scripts *environment.sh* and *build.sh* have been provided. The first script, *environment.sh*, appends the build directory to the `$PATH` variable, so that a 'production ready' environment can be tested.

- Run *environment.sh* once after starting your terminal emulator of choice. You don't need to run it again for this terminal session.
- Run cmake

### Windows
