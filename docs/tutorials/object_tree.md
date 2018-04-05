Object tree
===========

In SurgeScript, objects have a hierarchical relationship Suppose that object P spawns (or *instantiates*) object C. We say that object P is the *parent* of C. Similarly, object C is a *child* of P.

An object may spawn many children. However, all objects have only one parent. This relationship between objects form a hierarchy (or tree). The Figure below shows an example:

![An object tree](../img/tree.png)

In this example, Level is the parent of GameItem1 and GameItem2. Components A and B are children of GameItem1, but not of GameItem2. Level has two children and GameItem2 has zero.

In SurgeScript, the root of the hierarchy is a predefined object called **System**. Its parent is itself. All other objects are descendants of **System**.

Spawning objects
----------------

You can use *spawn()* to instantiate an object. Example:

```
object "Parent"
{
    child = spawn("Child");

    state "main"
    {
    }
}

object "Child"
{
    state "main"
    {
    }
}
```

Function *spawn()* may be invoked on objects other than the caller. In this case, the caller won't be the parent of the newly created object:

```
object "Parent"
{
    child = spawn("Child");
    grandChild = child.spawn("Child");

    state "main"
    {
    }
}
```

Destroying objects
------------------

Objects can be destroyed by calling *destroy()*. Whenever an object is destroyed, its children are destroyed as well.

```
object "Foo"
{
    state "main"
    {
        Console.print("This object does nothing.");
        destroy();
    }
}
```

Traversing the hierarchy
------------------------

Relevant data about the object hierarchy can be obtained using the following properties and functions:

Function / property|Description
-------------------|-----------
*obj.parent* (read-only) | The parent object
*obj.childCount* (read-only) | Number of immediate children
*obj.child(name)* | Gets a child object named *name*
*obj.findChild(name)* | Finds\* a descendant named *name*
*obj.children()* | Returns an Array with all the children of the object
*obj.sibling(name)* | Gets a sibling\*\* named *name*

> **Notes:**
>
> \* findChild() may be slow, as it performs a deep search within the object tree. One is encouraged to cache the result of this function (for example, in the object constructor) and to avoid using it in states or loops.
>
> \*\* We say that two distinct objects are siblings if they share the same parent.

Example:

```
object "Parent"
{
    child = spawn("Child");
    otherChild = spawn("Child");

    state "main"
    {
        Console.print("This object has " + childCount + " children.");
        destroy();
    }
}
```

The output is as follows:

```
This object has 2 children.
```