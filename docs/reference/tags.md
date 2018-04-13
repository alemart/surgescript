TagSystem
=========

Utility functions for dealing with object tags. This object is available at `System.tags` and shouldn't be instantiated directly.

Functions
---------

## list

`list()`

Lists all tags used in all objects.

*Returns*

This function spawns a new Array object with all tags and returns it.

## select

`select(tagName)`

Selects all objects that are tagged with `tagName`.

*Arguments*

* `tagName`: string.

*Returns*

This function returns a new Array with all the names of the objects tagged with `tagName`.