TagSystem
=========

Utility functions for dealing with object tags. This object is available at `System.tags` and shouldn't be instantiated directly.

Functions
---------

#### list

`list()`

Lists all tags used in all objects.

*Returns*

A new [Array](/reference/array) of strings with all tags used in all objects.

#### select

`select(tagName)`

Selects all objects that are tagged with `tagName`.

*Arguments*

* `tagName`: string.

*Returns*

A new [Array](/reference/array) of strings with the names of the objects tagged with `tagName`. If there are no such objects, an empty array is returned.

#### tagsOf

`tagsOf(objectName)`

Returns the tags of the objects named `objectName`.

*Available since:* SurgeScript 0.5.7

*Arguments*

* `objectName`: string.

*Returns*

A new [Array](/reference/array) of strings with the names of the tags associated with the objects named `objectName`. If the objects are untagged or do not exist, an empty array is returned.
