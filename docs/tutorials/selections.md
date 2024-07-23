Switch statement
================

Introduction
------------

The `switch` statement evaluates an expression and matches it against a sequence of `case` clauses. The statements following the matching `case` clause will be executed. Example:

```cs
// variable 'name' will be compared to "Surge", "Neon" and "Tux"
// the full name of the character will be printed to the screen
switch(name)
{
    case "Surge":
        Console.print("Surge the Rabbit");
        break;

    case "Neon":
        Console.print("Neon the Squirrel");
        break;

    case "Tux":
        Console.print("Tux the Penguin");
        break;
}
```

If no matching `case` clause is found, then the statements following the `default` clause, if present, will be executed. Example:

```cs
// we're checking if variable 'amount' is 1, 5, 20 or none of these
// depending on the amount, we'll print a different message to the screen
switch(amount)
{
    case 1:
        Console.print("Green rupee");
        break;

    case 5:
        Console.print("Blue rupee");
        break;

    case 20:
        Console.print("Red rupee");
        break;

    default:
        Console.print("Unknown rupee");
        break;
}
```
If no matching `case` clause is found and no `default` clause is present either, then all `case` sections will be skipped and the first statement following the `switch` block will be executed.

There must not be more than one `default` clause per `switch` block. It's usual practice to write the `default` section at the end of the `switch` block, though it's not a requirement.

*Note:* `switch` statements are supported since SurgeScript 0.6.1.

Breaking out
------------

In the above examples, there is a `break` statement at the end of each section. Use the `break` statement to break out of the body of the `switch` statement. Execution will continue at the first statement following the `switch` block. Example:

```cs
one = 1;

switch(one)
{
    case 1:
        Console.print("Number one");
        break;

    default:
        Console.print("This line will never be executed");
        break;
}

Console.print("We're done!");

//
// Output:
//
// Number one
// We're done!
//
```

!!! tip

    `break` statements placed before the end of a section have the same effect.

Multiple matching
-----------------

Sometimes it's useful to compare an expression to multiple values and run the same code if there is a match to any of these values. This may be done by writing multiple `case` clauses, one immediately after the other, as in the example below:

```cs
// let's check if amount is 1, 5, 20 or none of these
switch(amount)
{
    case 1:
    case 5:
    case 20:
        Console.print("This is a green, a blue or a red rupee.");
        break;

    default:
        Console.print("This is an unknown rupee.");
        break;
}
```

!!! info "Empty sections"

    Any `case` clause immediately followed by another `case` clause defines an empty section. In the above example, `case 1:` and `case 5:` define empty sections. It's not useful to have a `case` clause immediately followed by a `default` clause or vice-versa, though these too define empty sections.

Additional remarks
------------------

In a `switch` statement, the `case` keyword must be followed by a constant expression[^1], typically a number or a string literal. No two `case` expressions may evaluate to the same value.

The expression of the `switch` statement is first evaluated and then compared to the expressions of the `case` clauses. There is a match between these expressions if they are evaluated to the same value, in a [strong equality](expressions.md#comparison-operators) sense. This means that both the content and the type of the evaluated expressions must be the same for a match to occur.

In SurgeScript, all `case` and `default` sections **must** be terminated by a `break` or by a `return` statement, except when they are empty. While other languages such as C/C++ do not have this requirement, forgetting to terminate a section is often a source of bugs due to the [fallthrough behavior](https://en.wikipedia.org/wiki/Switch_statement#Fallthrough). In SurgeScript, the fallthrough behavior is disallowed, except in [empty sections](#multiple-matching).

[^1]: As of SurgeScript 0.6.1, only literals are supported.