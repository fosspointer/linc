# Linc Guide (#8): For Loops

Now that you've learnt the basics of how conditional logic works in Linc (utilizing `if` and `while` constructs), it's time to introduce `for` loops. Before that, however, think about this issue: What if I want to execute code an arbitrary amount of times? One approach might be to do the following:

```linc
#include `std.linc`

fn main() {
    println("print #0");
    println("print #1");
    println("print #2");
    ...
}
```

The above, however, can quickly become quite a cumbersome approach. What if you wanted to have 150 print statements? Strange though that might seem, it's not that uncommon in the grand scheme of things. After a bit of careful thinking, this is what you could come up with (try it for yourself first), utilizing variables and while loops:

```linc
#include `std.linc`

fn main() {
    counter: mut u32 = 0u; // Here, an unsigned number is used, since counts/amounts cannot be negative
    
    while counter < 15u32 {
        println("print #" + @counter); // Similar to the above example (except that the counter variable is now used)
        ++counter; // Increase the counter variable by 1
    };
}
```

This uses a while loop to execute the actual printing code, while limiting the amount of while iterations using a variable. In this case, the variable will have values 0-14, meaning the while loop will execute 15 times in total. You might be able to see that this type of logic is not only incredibly useful, but also very common too, as arbitrary repetition feels very fundamental. For this reason, there exists another construct: the `for` loop. For loops function similarly to while loops, having a certain condition that must be repeatedly satisfied. However, they also incorporate a variable declaration and a statement, so as to make this type of "counter logic" very trivial in the language in and of itself. It is arguably easier to manage such variables *inside* of their designated loops. This is how you define a basic `for` loop:

```linc
// Syntax: for(<var_decl> <condition> <statement>) <statement>
#include `std.linc`

fn main() {
    // This is functionally equivalent to the counter seen above
    for(counter: mut u32 = 0u counter < 15u32 ++counter;)
        println("print #" + @counter);
}
```

In this example, the parentheses are **not** used for grouping, but are mandatory in for loops, as every for loop's 'setup' consists of at least three parts. The first part, the variable declaration, is used to define a variable that exists within the loop's bounds. Additionally, the second part, the condition, functions the exact same as it does in a while loop. Lastly, the third and final part executes after every iteration, and is usually a modification of the for loop's variable (in this case, incrementing it).

Next: [Scoping](./9-scoping.md)
