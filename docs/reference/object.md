Object
======

In SurgeScript, all objects are also instances of `Object`. This means that the properties and functions listed below are available for all objects:

Properties
----------

* `parent`: object. Reference to the parent object.
* `childCount`: integer number. The number of children of this object.
* `active`: boolean, read-write. Indicates whether this object is active\* or not.
* `__name`: string. The name of this object (string).
* `__children`: Array object. This spawns a new array containing references to the children of this object.
* `__functions`: Array object. The functions of this object represented as a collection of strings.
* `__timespent`: number. The approximate time spent in this object in the last frame (in seconds).
* `__memspent`: number. The current memory consumption of this object (in bytes) - not implemented.

> **Note:**
>
> \* Objects are active by default. Whenever an object is set to be inactive, its state machine is paused. Additionally, the state machines of all its descendants are also paused.

Functions
---------

#### spawn

`spawn(objectName)`

Spawns an object named `objectName`.

*Arguments*

* `objectName`: string. The name of the object to be spawned / instantiated.

*Returns*

A new object of the desired name. Note that the newly created object will be a child of `this`.

#### destroy

`destroy()`

Destroys the object.

#### child

`child(childName)`

Looks for a child named `childName`.

*Arguments*

* `childName`: string. The name of the desired child.

*Returns*

A child of name `childName`, or `null` if there is no such object.

#### sibling

`sibling(siblingName)`

Looks for a sibling named `siblingName`. Two objects are siblings if they share the same parent.

*Arguments*

* `siblingName`: string. The name of the desired sibling.

*Returns*

A sibling of name `siblingName`, or `null` if there is no such object.

#### findObject

`findObject(objectName)`

Searches for a descendant (child, grand-child, and so on) named `objectName`. Since this function traverses the [object tree](../tutorials/object_tree), it's recommended to cache its return value. Do not use it in loops or states, as it might be slow.

*Arguments*

* `objectName`: string. The name of the desired object.

*Returns*

A descendant named `objectName`, or `null` if there is no such object.

*Example*

```
object "TestObject"
{
    // will search for SomeOtherObject in the Application
    obj = Application.findObject("SomeOtherObject");

    state "main"
    {
        if(obj != null)
            Console.print("Found the object.");
        else
            Console.print("Object not found.");

        destroy();
    }
}
```


#### timeout

`timeout(seconds)`

Checks if the object has been on the same state for more than a given amount of `seconds`.

*Arguments*

* `seconds`: number. The number of seconds.

*Returns*

Returns `true` if the object has been on the current state for more than `seconds` seconds.

*Example*

```
object "Time Test"
{
    state "main"
    {
        // Will wait for 2 seconds before changing the state
        if(timeout(2.0))
            state = "done";
    }

    state "done"
    {
        Console.print("Done!");
        destroy();
    }
}
```

#### toString

`toString()`

Converts the object to a string. This function is designed to be overloaded by your own objects.

*Returns*

A string.

#### equals

`equals(otherObject)`

Compares `this` object to `otherObject`. This function is designed to be overloaded by your own objects.

*Arguments*

* `otherObject`: object. An object to compare `this` to.

*Returns*

Returns `true` if the objects are equal; `false` otherwise.

#### hasFunction

`hasFunction(functionName)`

Checks if the object has a function named `functionName`.

*Arguments*

* `functionName`: string. The name of the function.

*Returns*

Returns `true` if the object has a function named `functionName`.

#### hasTag

`hasTag(tagName)`

Checks if the object has been tagged with `tagName`.

*Arguments*

* `tagName`: string. The name of the tag.

*Returns*

Returns `true` if the object has been tagged with `tagName`.