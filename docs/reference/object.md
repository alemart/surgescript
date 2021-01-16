Object
======

In SurgeScript, all objects are also instances of `Object`. This means that the properties and functions listed below are available for all objects:

Properties
----------

#### parent

`parent`: object, read-only.

Reference to the parent object.

#### __name

`__name`: string, read-only.

The name of the object.

#### __active

`__active`: boolean.

Indicates whether the object is active or not.

Objects are active by default. Whenever an object is set to be inactive, its state machine is paused. Additionally, the state machines of all its descendants are also paused.

#### __functions

`__functions`: [Array](/reference/array) object, read-only.

The functions of this object represented as a collection of strings.

#### __timespent

`__timespent`: number, read-only.

The approximate time spent in the current state (in seconds).

#### __file

`__file`: string, read-only.

The source file of this object.

*Available since:* SurgeScript 0.5.3

#### __children

`__children`: [Array](/reference/array) object, read-only.

The children of this object.

*Available since:* SurgeScript 0.5.4

*Returns*

A new array featuring all the children of this object. If there are no children, an empty array is returned.

#### __childCount

`__childCount`: number, read-only.

The number of children of the object.

*Available since:* SurgeScript 0.5.5

*Note:* prior to SurgeScript 0.5.5, you would use `object.childCount` instead. That form is now obsolete.


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

`child(childName | childId)`

Looks for a child named `childName` (or matching `childId`).

*Arguments*

* `childName`: string. The name of the desired child.
* `childId`: number. The id of the desired child, an integer between `0` and `childCount - 1`, inclusive.

*Returns*

The desired child, or `null` if there is no such object.

#### children

`children(childName)`

Finds all children named `childName`. Since this function spawns a new array at each call, it's recommended to cache its return value.

*Available since:* SurgeScript 0.5.4

*Arguments*

* `childName`: string. The name of the desired children.

*Returns*

A new array containing all children named `childName`. If no such children are found, an empty array is returned.

#### childWithTag

`childWithTag(tagName)`

Finds a child tagged `tagName`.

*Available since:* SurgeScript 0.5.4

*Arguments*

* `tagName`: string. The name of a tag.

*Returns*

A child tagged `tagName`, or `null` if there is no such object.

#### childrenWithTag

`childrenWithTag(tagName)`

Finds all children tagged `tagName`.

*Available since:* SurgeScript 0.5.4

*Arguments*

* `tagName`: string. The name of a tag.

*Returns*

A new array containing all children tagged `tagName`. If there are no such children, an empty array is returned.

#### sibling

`sibling(siblingName)`

Looks for a sibling named `siblingName`. Two objects are siblings if they share the same parent.

*Arguments*

* `siblingName`: string. The name of the desired sibling.

*Returns*

A sibling of name `siblingName`, or `null` if there is no such object.

#### findObject

`findObject(objectName)`

Finds a descendant (child, grand-child, and so on) named `objectName`. Since this function traverses the [object tree](/tutorials/object_tree), it's recommended to cache its return value. Do not use it in loops or states, as it might be slow.

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

#### findObjects

`findObjects(objectName)`

Finds all descendants named `objectName`. Since this function traverses the [object tree](/tutorials/object_tree), it's recommended to cache its return value. Do not use it in loops or states, as it might be slow.

*Available since:* SurgeScript 0.5.4

*Arguments*

* `objectName`: string. The name of the objects to be found.

*Returns*

A new array containing all descendants named `objectName`. If no such descendants are found, an empty array is returned.

#### findObjectWithTag

`findObjectWithTag(tagName)`

Finds a descendant tagged `tagName`. Since this function traverses the [object tree](/tutorials/object_tree), it's recommended to cache its return value. Do not use it in loops or states, as it might be slow.

*Available since:* SurgeScript 0.5.4

*Arguments*

* `tagName`: string. The name of a tag.

*Returns*

A descendant tagged `tagName`, or `null` if there is no such object.

#### findObjectsWithTag

`findObjectsWithTag(tagName)`

Finds all descendants tagged `tagName`. Since this function traverses the [object tree](/tutorials/object_tree), it's recommended to cache its return value. Do not use it in loops or states, as it might be slow.

*Available since:* SurgeScript 0.5.4

*Arguments*

* `tagName`: string. The name of a tag.

*Returns*

A new array containing all descendants tagged `tagName`. If no such descendants are found, an empty array is returned.

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

#### __invoke

`__invoke(functionName, paramsArray)`

Invokes function `functionName`, passing the parameters specified in `paramsArray`. Please note that the number of elements of `paramsArray` must be the same as the number of parameters required by the function to be invoked.

*Available since:* SurgeScript 0.5.2

*Arguments*

* `functionName`: string. The name of the function to be called.
* `paramsArray`: [Array](/reference/array). The parameters to be passed to the function.

*Returns*

Returns the value returned by the invoked function.

*Example*

```
// The program below will print:
// 12
// 12
// true

object "Application"
{
    state "main"
    {
        Console.print(this.sum(5, 7));
        Console.print(this.__invoke("sum", [5, 7]));
        Console.print(sum(5, 7) == __invoke("sum", [5, 7]));
        Application.exit();
    }

	fun sum(a, b)
	{
		return a + b;
	}
}
```

#### __arity

`__arity(functionName)`

Returns the number of arguments of function `functionName`, if it's defined. If it's not defined, zero will be returned.

*Available since:* SurgeScript 0.5.4

*Arguments*

* `functionName`: string. The name of a function.

*Returns*

The number of arguments of the specified function, or zero if the function is not defined.