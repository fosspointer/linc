# Linc Guide (#5): Input & Output

In the past chapters, the `println` function has been used for printing to the console for output. Having that in mind, let's look at Input/Output more in depth. 

But why isn't the function just called `print`? The answer to that is that there **is** a function called print, which prints its input to the console, similary to `println`. However, `println` **also** prints a new-line character (`'\n'`) after its given string, so as to ensure that no two messages appear on the same line. Here's an example program to clarify this further:

```linc
#include `std.linc`

fn main() {
    // This will print "helloworld" in a singular line, since we aren't using println
    print("hello")
    print("world")

    // Empty line for separation
    println("")

    // This will print "hello" and then "world" in the following line
    println("hello")
    println("world")
}
```

In this case, both `print` and `println` are functions that receive *expressions* as input, specifically of type `string` (this makes sense as strings represent arbitrary sequences of characters). User input can also be read from the console with the `reads` function.

When printing the value of an integer to the console, you might remember how the `@` operator was used. This operator converts its operand to a string, so that it can be printed using the usual print functions. For example, to convert the literal `5i32` to `"5"` you would write `@5i32`(notice how type information is insignificant in this case does not persist). The same holds true for variables, meaning that a variable `foo` can be converted to string, written as `@foo`.

Strings can also be combined, using the `+` operator (strings are concatenated similarly to how numbers are added). For example, we can get the string `"helloworld"` by adding `"hello" + "world"`. 

To exemplify everything demonstrated so far:

```linc
#include `std.linc`

fn main() {
    // Printing examples
    print("Enter your name: ") // here, print is used instead of println as the intent is to have the prompt in the same line as the console input (note how ": " was used to separate the two, without which entering a name would look like this: "Enter your nameMark")
    name: string = reads() // here, the reads() function is used to take input from the user

    println("Your name is " + name + '!') // here, we're concatenating the name variable with our string and printing all of it together (notice how characters can be appended to strings without the `@` operator)

    // Examples utilizing the `@` operator
    num: i32 = 10
    println("The number is " + @num) // -> "The number is 10"

    flag: bool = true
    println("The option is set to " + @flag) // -> "The option is set to true"

    // Showing its use in more complex expressions
    println("The result is " + @(num * 2))  // -> "The result is 20"
}
```

Next: [Conditionals in Linc](./6-conditionals.md)