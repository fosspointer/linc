# Linc Guide (#6): Conditionals

In the past chapters, we've taken a look at variables, input and output, mutability and expressions, which are all fundamental for programming in Linc. These are interesting on their own, but don't highlight how important and how dynamic programming really is, which poses the question: Everything thus far has just been computations and procedures. However, the programs I use in day to day life are **dynamic**, handling lots of different cases and conditions, and far more than just simple procedures. How is that possible?

The answer to that is that they make extensive use of **conditional logic**, namely **if expressions** and **while expressions** (the latter of which we'll get to in the next chapter). If expressions are powerful programming constructs whose function is very simple: check a value of a boolean and if it is `true` then execute a given piece of code. Alternatively, execute some other piece of code if it is `false` (optionally). Here's a program to showcase of how conditionals work:

```linc
#include `std.linc`

fn main() {
    name: string = reads()

    // Note that the `==` operator, unlike variable (re)assignment, simply checks two values and evaluates to true if they're equal (otherwise returning false)
    if name == "Zeus"
        println("You control lightning!")
    else if name == "Poseidon"
        println("You control the sea!")    
    else println("You are not a god :c") 
}
```

In this example, indentation is insignificant (which is always the case in the language), but generally a good idea to incorporate, so as to improve readability. Notice how in each case, there's only a singular statement after each condition, which is how conditionals work. However, it's possible to group multiple statements together using curly braces, like this:

```linc
#include `std.linc`

fn main() {
    print("Choose an option (y/n): ")
    option: string = reads()

    if option == "y" || option == "Y" { // remember: `||` is used to denote logical OR (it evaluates to true if at least one of the operands is true)
        println("you choose yes")
        println("<insert code that actually does soemthing here>)
    }
    else if option == "n" || option == "N" { // similarly to before, here we're chaining if expressions
        println("why did you even choose no?")
        println("bad idea")    
    }
    else println("you didn't even follow the instructions properly...")
}
```

Next: [While loops](./7-while_loops.md)