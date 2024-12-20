# Linc Guide (#4): Mutability

You've already learned the basics of how variables work in Linc. Still, imagine the case of a timer, where a variable stores the seconds elapsed from a specific point in time. As you can imagine, the value of this variable cannot be a constant, and must thus, have its value mutated (in this case every second). To achieve this, we can use the 'mut' data-type modifier, which as the name suggests, allows for data mutability. Here's a program to demonstrate this (note that `println(@<varname>)` is used to print the value of a variable, you can check [the first chapter](./1-hello_world.md) if needed):

```linc
#include `std.linc`

fn main() {
    my_variable: mut i32 = 0; // allows the variable to be modified
    println(@my_variable) // -> 0

    my_variable = 2; // here we reassigned the variable to the value 2
    println(@my_variable) // notice how the variable's value is now 2

    my_variable = my_variable + 1; // notice how this is not an equality! it's simply assignent of the variable to itself + 1, essentially incrementing it by 1

    // Since the above is really common, we can write increments like this:
    my_variable += 1;
    println(@my_variable) // we incremented the variable by 1 twice so it must equal 4 now

    my_variable /= 2; // we can use the same notation for subtraction, multiplication and division too
    println(@my_variable) // 4 / 2 = 2

    ++my_variable; // this is a shorthand for incrementing a variable by 1

    --my_variable; // similarly to the above, but for decrementing
}
```

Variables declared directly (using the `:=` specifier) can also be made mutable; here's an example:

```linc
fn main() {
    foo mut := 2; // foo is given the initial value of `2`
    bar mut := 3; // bar is given the initial value of `3`
    ++bar; // bar is increment by one, (now `4`)
    foo += bar; // bar is added to foo, which now has the value of `6`
}
```
Remember, variables whose types don't contain the `mut` keyword **cannot** be mutated.

Next: [Input/Output in Linc](./5-io.md)