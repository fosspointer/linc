# Linc Guide (#7): While Loops

Similarly to if expressions, while expressions are powerful conditional constructs, which are in reality just as essential as if expressions. Their key difference is the fact that their condition is repeatedly checked after each time the `true` statement executes, stopping once the condition becomes false. Interestingly enough, they also utilize the `else` constuct if the condition was never satisfied. Additonally, the `finally` construct is executed once after the loop stops, if the loop was true at least once. Here's an example to demonstrate this! 

```linc
#include `std.linc`

fn main() {
    while to_lower(readln("Enter a greeting: ")) == "hello" // user input is converted into lowercase such that "hello", "HELLO", "heLLO", etc... are all considered valid combinations (case insensitive)
        println("Goodbye!")
    else println("You didn't say hello!")
    finally println("You said hello at least once");
}
```

In this case, to_lower is a function that takes a string and makes it all lowercase (effectively making "heLLo!", "HELLO!" and "hello!" equivalent), which is used to make our input case insensitive. Then, if the while loop succeeds, the next statement (or in this case, printing the word goodbye) will be executed. Note that that both `else` and `finally` are optional, (you can specify either one of them, none, or both). However, if both are specified, they have to be ordered in this manner (first `finally`, then `else`).

Next: [For loops](./8-for_loops.md) 
