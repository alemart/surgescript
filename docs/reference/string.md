String
======

Routines for strings. The String object is not supposed to be used directly. The functions below are available for primitive values that are strings.

Example:

```
// Useful string routines
x = "SurgeScript".toLowerCase(); // "surgescript"
y = x.substr(0, 5); // "surge"
z = x[0]; // "s" (first character of x)
n = y.length; // 5
```

Please note that strings in SurgeScript are immutable. Once a string is set, its individual characters cannot be changed. If you need to modify the content of a string, reassign the variable to a new string.

Properties
----------

* `length`: number. The length of the string.

Functions
---------

#### valueOf

`valueOf()`

The primitive value of the string, i.e., the string itself.

*Returns*

The string.

#### toString

`toString()`

Convert to string.

*Returns*

The string itself.

#### equals

`equals(str)`

Compares the string to another string `str`.

*Arguments*

* `str`: string.

*Returns*

Returns `true` if the strings are equal.

#### get

`get(i)`

Gets the `i`-th character of the string. The `[ ]` operator can be used equivalently.

*Arguments*

* `i`: integer number. A value between 0 (inclusive) and the length of the string (exclusive).

*Returns*

The `i`-th character of the string (0-based index).

#### indexOf

`indexOf(str)`

Finds the position of the first occurrence of `str` in the string.

*Arguments*

* `str`: string. The string to be searched for.

*Returns*

The position (0-based index) of the first occurrence of `str` in the string, or *-1* if there is no such occurrence.

*Example*

```
name = "SurgeScript";
a = name.indexOf("Surge"); // a is 0
b = name.indexOf("Neon"); // b is -1
c = name.indexOf("e"); // c is 4
d = name.indexOf("script"); // d is -1, as the search is case-sensitive
```

#### substr

`substr(start, length)`

Extracts the substring starting at position `start` with length `length`.

*Arguments*

* `start`: number. The start position.
* `length`: number. The length of the substring.

*Returns*

The substring with length `length` starting at `start`.

*Example*

```
name = "SurgeScript";
surge = name.substr(0, 5); // "Surge"
script = name.substr(5, 6); // "Script"
e = name.substr(4, 2); // "e"
empty = name.substr(555, 1); // ""
```

#### concat

`concat(str)`

Concatenates two strings. This is the same as using the `+` operator.

*Arguments*

* `str`: string.

*Returns*

The caller string concatenated with `str` at the end.

*Example*

```
name = "Surge".concat("Script"); // SurgeScript
name = "Surge" + "Script"; // SurgeScript
```

#### replace

`replace(oldstr, newstr)`

Replaces all occurrences of `oldstr` to `newstr` in the caller string.

*Arguments*

* `oldstr`: string. The substring to be replaced.
* `newstr`: string. The substring that should appear in the result.

*Returns*

The caller string having all its occurrences of `oldstr` replaced to `newstr`.

*Example*

```
// dst is "Gimacian, Neon and Charge"
src = "Surge, Neon and Charge";
dst = src.replace("Surge", "Gimacian");
```

#### toLowerCase

`toLowerCase()`

Converts the string to lower case.

*Returns*

The string converted to lower case.

#### toUpperCase

`toUpperCase()`

Converts the string to upper case.

*Returns*

The string converted to upper case.