Expressions
===========

SurgeScript supports many kinds of expressions. Expressions may involve strings, numbers or booleans. Let's study each case:

Constants
---------

Constants are types of expressions. They may be numbers, strings, booleans... You may attribute constant values to variables using the assignment operator (`=`):

```cs
// strings
name = "Surge"; // variable name receives string "Surge"
message = "Surge's home";
otherMessage = 'Single-quoted strings are also allowed';

// numbers
weight = 35;
speed = 42.2;

// booleans
like = true;
crazy = false;

// null
next = null;
```

Arithmetic operators
--------------------

SurgeScript can perform basic arithmetic. The operators are as follows:

Operator|Meaning
--------|-------
`+` | Addition
`-` | Subtraction
`*` | Multiplication
`/` | Division
`%` | Remainder

The multiplication, division and remainder operators have precedence over the addition and the subtraction operators.

Examples:

```cs
two = 1 + 1;
ten = 5 * 2;
eleven = 5 * 2 + 1;
four = two * two;
fifteen = eleven + four;
three = fifteen / 5;
positive = +5;
negative = -1;
zero = 15 - 10 - 5;
one = 15 - (10 + 4);
twelve = (2 + 2) * 3;
prime = -(-15 + 8);
seven = 5 + 5 % 3;
negativeTwo = -5 % 3;
```

Comparison operators
--------------------

Comparison operators compare two operands.

Operator|Meaning
--------|-------
`==` | Equality operator
`!=` | Inequality operator
`<` | Less than
`<=` | Less or equal
`>` | Greater than
`>=` | Greater or equal
`===` | Strong equality
`!==` | Strong inequality

Examples:

```cs
x = 1;
y = (x == 1); // y is true
z = (x != 1); // z is false
w = x > 1; // w is false
t = x >= 1; // t is true
```

!!! warning "Pay attention!"

    Notice that the equality operator, used for comparing expressions for equality, is `==`. Pay attention! Don't confuse the equality operator (`==`) with the assignment operator (`=`). The expression `(x = 1)` always evaluates to 1, whereas `(x == 1)` is `true` only if x equals 1.

!!! info "Strong equality"

    Unlike their counterparts, strong equality/inequality operators compare not only the content of the operands, but also their type. While `"1" == 1` evalutes to `true`, `"1" === 1` does not.

Logical operators
-----------------

Logical operators involve booleans (`true` or `false` values). While the **not** operator requires only one operand, operators **and** and **or** require two.

Operator|Meaning
--------|-------
`&&` | and
`||` | or
`!` | not

The table below summarizes the basic logical expressions:

Expression|Result
----------|------
`a && b` |`true` only if both `a` and `b` are true
`a || b` | `true` if `a` is true or `b` is true
`!a` | `true` if `a` is false, `false` if `a` is true

Notice that **not** has higher precedence than the other two operators. Examples:

```cs
x = 5 < 2; // evaluates to false
y = !x; // evaluates to true
z = 18 < 35 || x; // evaluates to true
w = (z && x) || true; // evaluates to true
t = !x && x; // evaluates to false
```

!!! info "Short-circuit evaluation"

    SurgeScript performs short-circuit evaluation in **and** and **or** operations. This means that the second operator is not evaluated if the first alone is sufficient to determine the value of the operation. Example: in the expression `a || b`, expression `b` is not evaluated if `a` is evaluated to `true`. Similarly, in `a && b`, expression `b` is not evaluated if `a` is evaluated to `false`.

Textual expressions
-------------------

You may concatenate strings with the `+` operator. Example:

```cs
message = "Surge" + " rocks!"; // evaluates to "Surge rocks!"
```

Other data types, such as numbers and booleans, may also be implicitly converted to strings when using this feature. Example:

```cs
group = "Jackson " + 5; // evaluates to "Jackson 5"
```

Assignment operators
--------------------

The attribution operator (`=`) may be used to attribute data to variables. However, there's more to it. The expression `variable = "data"` is itself evaluated to string `"data"`.

Likewise, the expression `variable = variable + 5` is evaluated to the new value of the variable and may be shortened to `variable += 5`.

Let's see what kind of assignment expressions we have:

Expression|Translates to|Meaning
----------|-------------|-------
`x = value`|`x = value`|Assigns `value` to `x`
`x += value`|`x = x + value`|Adds `value` to `x`
`x -= value`|`x = x - value`|Subtracts `value` from `x`
`x *= value`|`x = x * value`|Multiplies `x` by `value`
`x /= value`|`x = x / value`|Divides `x` by `value`
`x %= value`|`x = x % value`|Sets `x` to the remainder of `x / value`

Example:

```cs
language = "Surge";
language += "Script";
// language now evaluates to "SurgeScript"

number = 5;
number += 2;
// number now evaluates to 7
```

Increment and decrement
-----------------------

Increment and decrement operators should be used only on numeric variables.

Expression|Meaning
----------|-------
`x++`|Increments x by 1 and returns the old value of x
`x--`|Decrements x by 1 and returns the old value of x
`++x`|Increments x by 1 and returns the new value of x
`--x`|Decrements x by 1 and returns the new value of x

Example:

```cs
x = 1;   // x is 1
y = x++; // y is 1
z = ++x; // z is 3
w = --x; // w is 2
t = --x; // t is 1
Console.print(x); // prints 1
```

Chained expressions
-------------------

You may chain assignment operations:

```cs
x = y = z = 1;
```

Commas
------

The comma operator takes two operands. It first evaluates the first operand and then discards the result. Next, it evaluates the second operand and then returns this result. The comma is the operator with the lowest precedence.

```cs
c = (1, 2); // Results in c = 2
a = 1, b = 2, c = 3; // Results in a = 1, b = 2 and c = 3
d = (c++, a); // Results in c = 4, d = 1
e = (++a, f = 2, 3); // Results in a = 2, e = 3, f = 2
```


Other expressions
-----------------

#### this

The keyword `this` represents the object in which it appears. Example:

```cs
this.destroy(); // destroys this object
```

#### typeof

The expression `typeof(expr)` (or simply `typeof expr`) is evaluated to a string corresponding to the type of `expr`. The possible types are: *number*, *string*, *boolean*, *object* or *null*. Example:

```cs
t = typeof 5; // t will hold the string "number"
```

#### timeout

The expression `timeout(seconds)` is a handy feature of SurgeScript. It is evaluated to `true` if the object has been on the same state for the specified amount of time or more. It can only be used inside states. Example:

```cs
object "Application"
{
    state "main"
    {
        if(timeout(2.0)) {
            Console.print("2 seconds have passed");
            state = "done";
        }
    }

    state "done"
    {
    }
}
```