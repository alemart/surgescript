Mouse
=====

The Mouse object is used to read input from the mouse.

*Example*

```
// This example shows how to use a sprite
// as a mouse cursor. Since My Cursor is
// tagged "detached", it will be rendered
// in screen space, not in world space.

using SurgeEngine.Actor;
using SurgeEngine.Transform;
using SurgeEngine.Input.Mouse;

object "My Cursor" is "entity", "detached"
{
    actor = Actor("My Cursor");
    transform = Transform();

    state "main"
    {
        transform.position = Mouse.position;
        if(Mouse.buttonPressed("left"))
            Console.print("left click");
    }

    fun constructor()
    {
        actor.zindex = 1.0;
    }
}
```

Properties
----------

#### position

`position`: [Vector2](/engine/vector2) object, read-only.

The position of the mouse cursor, in screen space.

Functions
---------

#### buttonDown

`buttonDown(buttonName)`

Checks if a mouse button is currently being held down.

*Arguments*

* `buttonName`: string. One of the following: *"left"*, *"right"*, *"middle"*.

*Returns*

Returns `true` if the specified button is being held down.

#### buttonPressed

`buttonPressed(buttonName)`

Checks if a mouse button has just been pressed.

*Arguments*

* `buttonName`: string. One of the following: *"left"*, *"right"*, *"middle"*, *"wheelUp"*, *"wheelDown"*.

*Returns*

Returns `true` if the specified button has just been pressed (i.e., a click has just occurred).

*Example*
```
using SurgeEngine.Input.Mouse;
using SurgeEngine.Collisions.CollisionBall;
using SurgeEngine.Transform;
using SurgeEngine.Camera;

// Use the mouse cursor to click on any entity that has a collider.
// Setup: simply place this object on the level.
object "Entity Picker" is "awake", "entity"
{
    transform = Transform();
    collider = CollisionBall(8);

    state "main"
    {
        transform.position = Camera.screenToWorld(Mouse.position);
        collider.visible = true;
    }

    fun onOverlap(otherCollider)
    {
        if(Mouse.buttonPressed("left"))
            Console.print("Clicked on " + otherCollider.entity.__name);
    }
}
```

*Note:* options *"wheelUp"* and *"wheelDown"* are available since Open Surge 0.5.1.

#### buttonReleased

`buttonReleased(buttonName)`

Checks if a mouse button has just been released.

*Arguments*

* `buttonName`: string. One of the following: *"left"*, *"right"*, *"middle"*.

*Returns*

Returns `true` if the specified button has just been released.
