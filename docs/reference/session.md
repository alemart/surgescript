Session
=======

This Dictionary-like object store session variables: variables that persist throughout the entire Application. Session variables are available at `Application.session`.

Usage:

```
// number of lives
Application.session["lives"] = 3;

...

// game over
if(Application.session["lives"] == 0)
    gameOver();
```

Properties
----------

* `__data`: [Dictionary](dictionary) object. Internal representation of the session variables.

Functions
---------

#### get

`get(variableName)`

Gets the value of the specified session variable. The `[ ]` operator may be used equivalently.

*Arguments*

* `variableName`: string. The name of the variable.

*Returns*

The value of the specified variable (it can be of any type), or `null` if there is no such variable.

#### set

`set(variableName, value)`

Sets a new value to the specified session variable. The `[ ]` operator may be used equivalently.

*Arguments*

* `variableName`: string. The name of the variable.
* `value`: any type. The new value of the variable.

#### isset

`isset(variableName)`

Checks if a session variable has been set.

*Arguments*

* `variableName`: string. The name of the variable.

*Returns*

Returns `true` if the specified variable has been set; or `false` otherwise.

#### unset

`unset(variableName)`

Erases a certain session variable from the memory.

*Arguments*

* `variableName`: string. The name of the variable.

#### clear

`clear()`

Erases all session variables.

#### toString

`toString()`

Converts the set of session variables to a string, in a Dictionary-like format.

*Returns*

The set of session variables converted to a string.