Mouse
=====

The Mouse object is used to read input from the mouse.

Example
-------
```
// This example shows how to use the Collectible
// sprite as a mouse cursor. Since MyCursor is
// tagged as "detached", it will be rendered in
// screen space (not world space).
using SurgeEngine.Mouse;
using SurgeEngine.Actor;

object "MyCursor" is "entity", "detached"
{
    actor = Actor("MyCursor");

    state "main"
    {
        actor.transform.xpos = Mouse.xpos;
        actor.transform.ypos = Mouse.ypos;
        if(Mouse.buttonPressed(0))
            Console.print("left click");
    }
}

object "Application"
{
    mouseCursor = spawn("MyCursor");

    state "main"
    {
    }
}
```

Properties
----------

* `xpos`: number, read-only. The x-position of the mouse cursor, in screen space.
* `ypos`: number, read-only. The y-position of the mouse cursor, in screen space.

Functions
---------

#### buttonDown

`buttonDown(buttonId)`

Checks if a mouse button is currently being held down.

*Arguments*

* `buttonId`: number. One of the following: 0 (left button), 1 (right button) or 2 (middle button).

*Returns*

Returns `true` if the specified button is being held down.

#### buttonPressed

`buttonPressed(buttonId)`

Checks if a mouse button has just been pressed.

*Arguments*

* `buttonId`: number. One of the following: 0 (left button), 1 (right button) or 2 (middle button).

*Returns*

Returns `true` if the specified button has just been pressed (i.e., a click has just occurred).

#### buttonReleased

`buttonReleased(buttonId)`

Checks if a mouse button has just been released.

*Arguments*

* `buttonId`: number. One of the following: 0 (left button), 1 (right button) or 2 (middle button).

*Returns*

Returns `true` if the specified button has just been released.
