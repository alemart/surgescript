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

Selects all objects that are tagged `tagName`.

*Arguments*

* `tagName`: string.

*Returns*

A new [Array](/reference/array) of strings with the names of the objects tagged with `tagName`. If there are no such objects, an empty array is returned.

#### tagsOf

`tagsOf(objectName)`

Returns the tags of the objects named `objectName`.

*Available since:* SurgeScript 0.6.0

*Arguments*

* `objectName`: string.

*Returns*

A new [Array](/reference/array) of strings with the names of the tags associated with the objects named `objectName`. If the objects are untagged or do not exist, an empty array is returned.

#### hasTag

`hasTag(objectName, tagName)`

Checks if objects named `objectName` are tagged `tagName`. Prefer using [Object.hasTag()](/reference/object#hastag) if possible.

See also: [Object.hasTag()](/reference/object#hastag).

*Available since:* SurgeScript 0.6.0

*Arguments*

* `objectName`: string.
* `tagName`: string.

*Returns*

Returns `true` if the objects are tagged as specified, or `false` otherwise.
