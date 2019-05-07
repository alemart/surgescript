System
======

The root object. Used to gather data from the language and to access special objects. It can be accessed by typing `System`.

Properties
----------

#### tags

`tags`: [Tag System](/reference/tags) object, read-only.

A reference to the Tag System.

#### gc

`gc`: [Garbage Collector](/reference/gc) object, read-only.

A reference to the Garbage Collector object.

#### objectCount

`objectCount`: number, read-only.

The total number of objects at this moment.

Functions
---------

#### exit

`exit()`

Exits the Application.

#### destroy

`destroy()`

The same as `exit()`.