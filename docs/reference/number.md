Number
======

Routines for numbers. The Number object is not supposed to be used directly. The functions below are available for primitive values of the number type.

Example:

```
x = 2.toString(); // x is "2"
t = typeof(x); // t is "string"
```

Functions
---------

#### valueOf

`valueOf()`

The primitive value of the number, i.e., the number itself.

*Returns*

The number.

#### toString

`toString()`

Convert number to string.

*Returns*

The number converted to a string.

#### equals

`equals(x)`

Compares the number to another number `x`. Since this routine performs a comparison between floating point numbers, it's recommended to use [Math.approximately()](math#approximately) instead.

*Arguments*

* `x`: number. The value to compare the number to.

*Returns*

Returns `true` if the numbers are equal.