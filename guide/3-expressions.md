# Linc Guide (#3): Expressions in Linc

Programming languages, just like real ones follow certain rules, which you might have already noticed. In the majority of programming languages, including Linc, all "structures" are one of three categories: expressions, statements and declarations. In the case of variables, they are **declarations** (that's why **declare** a variable), which take **expressions** as values. Practically, an expression is any string of code that results in a value of a certain data type. As you've seen in previous sections of this guide, numbers such as `5`, `7i32`, `-.5f64`, `"hi"`, `'a'` are all valid expressions. These are called literals, because they describe *literal* primitive values, and not the interactions/combinations thereof. However, there's much more to expressions than this. For example, here's how you can describe basic arithmetic operations in Linc:

```linc

some_number: i32 = 1 + 1 // -> 2
some_number2: i32 = 1 + 2 * 3 // -> 7 (the order of operations still persists in linc, meaning that multiplication (`*`) has higher precedence than addition)
some_number3: i32 = (1 + 2) * 3 // -> 9 (we can use parentheses to group exprssions!)
some_number4: i32 = -8 / 3 // -> -2 (the result gets rounded down since we defined it as an integer)

some_other_number: f32 = 8f / 3f // -> 2.666.. (in this case, we **do** have access to "decimal" numbers, since we used a floating point data type)
```

These symbols that we use to describe such operations are called **operators**, namely **binary operators**, which *"operate"* on pairs of values (such as addition/subtraction/multiplication/division, e.g. 2 * 3), and **unary operators** (like negation, e.g. -(-7)), which only *"operate"* on singular values. In programming, however, it's very common useful to use boolean operators, i.e. those that have boolean values (true or false) as operands, as those are often used in conditionals (we will get to those later). Below is an example utilizing boolean operations:

```linc
some_boolean: bool = true && true // -> true: this is the logical AND operator, which results in true ONLY if both operands are true
some_boolean2: bool = false || false // -> false: this is the logical OR operator, which results in true if AT LEAST one operand is true
some_boolean3: bool = !false // -> true: this is the logical NOT operator, which results in the opposite of its operand (i.e. true for false and false for true)

more_complicated_example: bool = (true || false) && !false // can you figure out what this results in?
```

Notice how up to now, all simple and "compound" expressions shown have been constructed using **literal** values (i.e. numbers and keywords). One might ask: variables wouldn't really be useful if storing data was on hand hand possible, but not accessing it on the other hand. That's why every variable's identifier (name) is a valid expression, one equivalent to the value stored within it. Here's an example to demonstrate this:

```linc
pi: f32 = 3.14159f32 // a basic approximation of π
two_pi: f32 = pi * 2f32 // here using the value stored within the `pi` variable
half_pi: f32 = pi / 2f32 // it's possible to use the value of a variable an arbitrary amount of times

some_variable_involving_pi: f32 = pi * 3f / 2f + (pi - 2f) * 9f // we can use the 'pi' variable just like an ordinary value
```

Next: [Mutability in Linc](./4-mutability.md)