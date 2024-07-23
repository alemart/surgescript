Conditionals
============

Introduction
------------

Conditionals are `if-then` statements. If a certain `condition` evaluates to `true`, execute a block of code. If not, don't execute it.

```cs
if(condition) {
    // this will be executed only if
    // the condition is true
}
```

Alternatively, we may include an `else` statement followed by a block of code that will only be executed if the condition is **not** true:

```cs
if(condition) {
    // this will be executed only if
    // the condition is true
}
else {
    // this will be executed only if
    // the condition is false
}
```

Example
-------

The following example will print *underaged* if variable `age` is less than 18, or *adult* otherwise:

```cs
object "Application"
{
    age = 23;

    state "main"
    {
        if(age < 18) {
            // variable age is less than 18
            Console.print("underaged");
        }
        else {
            // variable age is not less than 18
            Console.print("adult");
        }
    }
}
```


Inline conditionals
-------------------

Just like other languages with C-based syntax, the expression `condition ? true_value : false_value` evaluates to `true_value` if `condition` is `true` and to `false_value` if `condition` is `false`.

For example, the script below will print *underaged* if variable `age` is less than 18, or *adult* otherwise:

```cs
object "Application"
{
    age = 23;
    message = age < 18 ? "underaged" : "adult";

    state "main"
    {
        Console.print(message);
    }
}
```
