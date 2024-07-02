# Linc Guide (#1): Your first program

Like in the majority of programming languages, linc uses a unique file extension to identify it and differentiate it from code in other languages. In particular, the `.linc` extension is used (i.e. a simple program might be named `hello.linc`).

It is suggested to create a development directory inside of your home directory, in order to keep all code organized, which is really helpful. On Linux and Windows PowerShell, you can use the following commands (inside the terminal) to achieve this:

```sh
$ mkdir ~/dev
$ cd ~/dev
```

Now let's create your first program in Linc: one that prints the words "Hello, World!" to the console; a typical first program for all beginners in a language. Inside your development directory, create a file named `hello.linc` and enter the following code as its contents:

```linc
#include `std.linc`

fn main() {
    println("Hello, World!")
}
```

You can check this check and execute this program by running the following command inside your terminal (in your development directory):

```sh
$ lincenv hello.linc
```

You should now see the words "Hello, World!" displayed on your screen. You just wrote your first program in Linc. Now, let's break down how it works:

- ```#include `std.linc` ```: This line of code makes the `std` header available to our program, which stands for standard, meaning Linc's **standard library**. Practically, it provides basic utilities for input/output, basic math functions, etc... (in this case, `println` was useed). This header is available to all Linc installs by default, so there's no requirements to be able to use it.

- `fn main() { ... }`: This is a function that defines our 'entry-point'. It's how Linc can know where our program starts and what code needs to execute. For this reason, the name `main` must be used. Functions will be explained more in depth in later lessons.

- `println("Hello, World!")`: This achieves the actual job of printing our "hello world" message. As for the "ln" part, a new-line character is appended to the end of the message, so that any following printing can happen at the beginning of its own line (the code is functionally equivalent to ```print("Hello, World!\n")```, as the new-line character is represented as `\n`).

Next: [Introduction to Variables](2-variables_introduction.md)