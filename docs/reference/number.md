Number
======

Routines for numbers. The Number object is not supposed to be used directly. The functions below are available for primitive values of the number type.

Example:

```
x = 2.toString(); // x is "2"
t = typeof(x); // t is "string"
```

**Note:** the SurgeScript Runtime stores numbers as 64-bit floating point values (following the IEEE 754 standard). Integer numbers are accurate up to 15 digits.

Functions
---------

#### valueOf

`valueOf()`

The primitive value of the number, i.e., the number itself.

*Returns*

The number.

#### toString

`toString()`

Converts the number to a string.

*Returns*

The number converted to a string.

#### equals

`equals(x)`

Compares the number to another number `x`. Since this routine performs a comparison between floating point numbers, it's recommended to use [Math.approximately()](math#approximately) instead.

*Arguments*

* `x`: number. The value to compare the number to.

*Returns*

Returns `true` if the numbers are equal.

#### isFinite

`isFinite()`

Checks if the number is finite.

*Available since:* SurgeScript 0.5.2

*Returns*

Returns `true` if the number is finite.

#### isNaN

`isNaN()`

Checks if the value is NaN (Not-a-Number).

*Available since:* SurgeScript 0.5.2

*Returns*

Returns `true` if the value is NaN.

#### isInteger

`isInteger()`

Checks if the number is an integer.

*Available since:* SurgeScript 0.5.2

*Returns*

Returns `true` if the number is an integer.