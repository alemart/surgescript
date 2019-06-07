Math
====

Built-in mathematical functions. The Math object can be accessed simply by typing `Math`.

Examples:

```
// Math functions
two = Math.sqrt(4); // square root of 4
eight = Math.pow(2, 3); // 2 raised to the 3rd power
zero = Math.sin(0); // sine of 0
seven = Math.max(5, 7); // maximum of 5 and 7
```

Properties
----------

#### pi

`pi`: number, read-only.

An approximation of pi (3.1415926535...)

#### epsilon

`epsilon`: number, read-only.

The difference between 1 and the smallest floating point number that is greater than 1.

#### infinity

`infinity`: number, read-only.

A floating point representation of positive infinity.

#### NaN

`NaN`: number, read-only.

A numeric data type representing Not-a-Number.

*Available since:* SurgeScript 0.5.3

Functions
---------

#### sqrt

`sqrt(x)`

Square root function.

*Arguments*

* `x`: number.

*Returns*

The square root of `x`.

#### pow

`pow(x, p)`

Raise to power.

*Arguments*

* `x`: number. The base.
* `p`: number. The exponent.

*Returns*

Returns `x` raised to the `p` power.

#### exp

`e(x)`

Exponential function.

*Arguments*

* `x`: number.

*Returns*

Returns *e* raised to the `x` power.

#### log

`log(x)`

Natural logarithm.

*Arguments*

* `x`: number.

*Returns*

The natural logarithm (base *e*) of `x`.

#### log10

`log10(x)`

Base 10 logarithm.

*Arguments*

* `x`: number.

*Returns*

The base 10 logarithm of `x`.

#### sin

`sin(x)`

Sine function.

*Arguments*

* `x`: number. Angle in radians.

*Returns*

The sine of `x`.

#### cos

`cos(x)`

Cosine function.

*Arguments*

* `x`: number. Angle in radians.

*Returns*

The cosine of `x`.

#### tan

`tan(x)`

Tangent function.

*Arguments*

* `x`: number. Angle in radians.

*Returns*

The tangent of `x`.

#### asin

`asin(x)`

Arc-sine function.

*Arguments*

* `x`: number.

*Returns*

The angle in radians whose sine is `x`.

#### acos

`acos(x)`

Arc-cosine function.

*Arguments*

* `x`: number.

*Returns*

The angle in radians whose cosine is `x`.

#### atan

`atan(x)`

Arc-tangent function.

*Arguments*

* `x`: number.

*Returns*

The angle in radians whose tangent is `x`.

#### atan2

`atan2(y, x)`

Function atan2.

*Arguments*

* `y`: number.
* `x`: number.

*Returns*

The angle, in radians, between the positive x-axis and the *(x, y)* vector.

#### deg2rad

`deg2rad(degrees)`

Converts `degrees` to radians.

*Available since:* SurgeScript 0.5.3

*Arguments*

* `degrees`: number.

*Returns*

The converted value.

#### rad2deg

`rad2deg(radians)`

Converts `radians` to degrees.

*Available since:* SurgeScript 0.5.3

*Arguments*

* `radians`: number.

*Returns*

The converted value.

#### random

`random()`

Random value.

*Returns*

A random number between 0 (inclusive) and 1 (exclusive).

#### floor

`floor(x)`

Floor function.

*Arguments*

* `x`: number.

*Returns*

The largest integer less or equal than `x`.

#### ceil

`ceil(x)`

Ceiling function.

*Arguments*

* `x`: number.

*Returns*

The smallest integer greater or equal than `x`.

#### round

`round(x)`

Round to the nearest integer.

*Arguments*

* `x`: number.

*Returns*

Returns `x` rounded to the nearest integer.

If the fraction of `x` is 0.5, this function uses the [commercial rounding](https://en.wikipedia.org/wiki/Rounding#Round_half_away_from_zero) tie-breaking method.

#### mod

`mod(x, y)`

Modulus function. Used to get the remainder of a division.

*Deprecated since SurgeScript 0.5.3. Use the remainder operator (%) instead, e.g.* `x % y`.

*Arguments*

* `x`: number.
* `y`: number.

*Returns*

The remainder of `x / y`.

#### sign

`sign(x)`

The sign of `x`.

*Arguments*

* `x`: number.

*Returns*

Returns 1 if `x` is positive or zero; or -1 if `x` is negative.

#### abs

`abs(x)`

Absolute value of `x`.

*Arguments*

* `x`: number.

*Returns*

Returns `-x` if `x` is negative, or `x` otherwise.

#### min

`min(x, y)`

The minimum of two values.

*Arguments*

* `x`: number.
* `y`: number.

*Returns*

Returns the smallest of two values: `x` and `y`.

#### max

`max(x, y)`

The maximum of two values.

*Arguments*

* `x`: number.
* `y`: number.

*Returns*

Returns the largest of two values: `x` and `y`.

#### clamp

`clamp(val, min, max)`

Clamps a value between a minimum and a maximum.

*Arguments*

* `val`: number. The value to be clamped.
* `min`: number. Minimum value.
* `max`: number. Maximum value.

*Returns*

Returns `val` clamped between `min` and `max`. Function `clamp` behave as follows:

* if `val` < `min`, it returns `min`
* if `val` > `max`, it returns `max`
* otherwise, it returns `val`

#### approximately

`approximately(x, y)`

Compares two floating point values. Since comparing two floating point numbers for equality directly may result in inaccuracies, this is a handy function.

*Arguments*

* `x`: number.
* `y`: number.

*Returns*

Returns `true` if `x` and `y` are *"approximately"* equal.

#### lerp

`lerp(a, b, t)`

Linear interpolation.

*Arguments*

* `a`: number.
* `b`: number.
* `t`: number. A value between 0 and 1.

*Returns*

Returns the linear interpolation between `a` and `b` by `t`.

Value `t` is clamped automatically to range *[0, 1]*.

As an example, note that `lerp`:

* returns `a` if `t` is *0*
* returns `b` if `t` is *1*
* returns `(a + b) / 2` if `t` is *0.5*

#### smoothstep

`smoothstep(a, b, t)`

Interpolation smoothing at the limits.

*Arguments*

* `a`: number.
* `b`: number.
* `t`: number. A value between 0 and 1.

*Returns*

Returns an interpolated value between `a` and `b` by `t`, in a similar way to `lerp`. Unlike the linear `lerp`, `smoothstep` is smooth at the limits. This is useful to create smooth transitions and animations.

Value `t` is clamped automatically to range *[0, 1]*.
