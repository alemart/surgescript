Expressions
===========

SurgeScript supports many kinds of expressions. Expressions may involve strings, numbers or booleans. Let's study each case:

Constants
---------

Constants are types of expressions. They may be numbers, strings, booleans... You may attribute constant values to variables using the assignment operator (*=*):

```
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
+ | Addition
- | Subtraction
* | Multiplication
/ | Division

Examples:

```
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
```

Comparison operators
--------------------

Comparison operators compare two operands.

Operator|Meaning
--------|-------
== | Equality operator
!= | Inequality operator
< | Less than
<= | Less or equal
\> | Greater than
\>= | Greater or equal

Examples:

```
x = 1;
y = (x == 1); // y is true
z = (x != 1); // z is false
w = x > 1; // w is false
t = x >= 1; // t is true
```

> **Pay attention!**
>
> Notice that the equality operator, used for comparing expressions for equality, is *==*. Pay attention! Don't confuse the equality operator (*==*) with the assignment operator (*=*). The expression `(x = 1)` always evaluates to 1, whereas `(x == 1)` is *true* only if x equals 1.

Logical operators
-----------------

Logical operators involve booleans (*true* or *false* values). While the **not** operator requires only one operand, operators **and** and **or** require two.

Operator|Meaning
--------|-------
&& | and
\|\| | or
! | not

The table below summarizes the basic logical expressions:

Expression|Result
----------|------
`a && b` |*true* only if both a and b are true
`a || b` | *true* if either a or b are true (or both)
`!a` | *true* if a is false, *false* if a is true

Notice that **not** has higher precedence than the other two operators. Examples:

```
x = 5 < 2; // evaluates to false
y = !x; // evaluates to true
z = 18 < 35 || x; // evaluates to true
w = (z && x) || true; // evaluates to true
t = !x && x; // evaluates to false
```

Textual expressions
-------------------

You may concatenate strings with the *+* operator. Example:

```
message = "Surge" + " rocks!"; // evaluates to "Surge rocks!"
```

Other data types, such as numbers and booleans, may also be implicitly converted to strings when using this feature. Example:

```
group = "Jackson " + 5; // evaluates to "Jackson 5"
```

Assignment operators
--------------------

The attribution operator (*=*) may be used to attribute data to variables. However, there's more to that. The expression `variable = "data"` is itself evaluated to string *"data"*.

Likewise, the expression `variable = variable + 5` is evaluated to `variable + 5` and may be shortened to `variable += 5`.

Let's see what kind of assignment expressions we have:

Expression|Translates to|Meaning
----------|-------------|-------
`x = value`|`x = value`|Assigns *value* to x
`x += value`|`x = x + value`|Adds *value* to x
`x -= value`|`x = x - value`|Subtracts *value* from x
`x *= value`|`x = x * value`|Multiplies x by *value*
`x /= value`|`x = x / value`|Divides x by *value*

Example:

```
language = "Surge";
language += "Script";
// language now evaluates to "SurgeScript"
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

```
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

```
x = y = z = 1;
```

Using commas will evaluate all expressions between then and return the right-most one:

```
x = 1; // x is 1
y = x++, 2; // y is 2
z = y--, --x; // z is 1
// x, y and z end up equal to 1
```


Other expressions
-----------------

#### typeof

The expression `typeof expr` evaluates to a string informing the type of `expr`. Example:

```
t = typeof 5; // t will hold the string "number"
```

The possible types are: *number*, *string*, *boolean*, *null*, *object*.

#### this

The keyword `this` represents the object in which it appears. Example:

```
this.destroy(); // destroys this object
```