Camera
======

The Camera object can be used to control what content (in world space) is rendered to the screen. The Camera is represented by a 2-dimensional point in world space that is mapped to the center of the screen.

> **HUD elements**
>
> Objects tagged as *"detached"* do not follow the camera. They are not in world space, but in screen space. This is useful for creating HUD elements.

*Example*

```
// Import the Camera object
using SurgeEngine.Camera;

// Reading the Camera status
// This object prints the position of the camera at every second
object "Camera Status" is "awake", "entity"
{
    state "main"
    {
        if(timeout(1))
            state = "print";
    }

    state "print"
    {
        Console.print(Camera.position);
        state = "main";
    }
}
```

Properties
----------

#### position

`position`: [Vector2](/engine/vector2) object.

The position of the camera in world space.

#### locked

`locked`: boolean, read-only.

Is the camera locked to a certain area in space? Defaults to `false`.

Functions
---------

#### lock

`lock(left, top, right, bottom)`

Locks the camera to a certain rectangular area in space. All coordinates are given in pixels and represent the boundaries of the rectangular area. They are such that `left` <= `right` and `top` <= `bottom`.

*Arguments*

* `left`: number. A x-coordinate in world space.
* `top`: number. A y-coordinate in world space.
* `right`: number. A x-coordinate in world space.
* `bottom`: number. A y-coordinate in world space.

#### unlock

`unlock()`

Unlocks the camera. If unlocked, the camera moves freely throughout the space.

#### worldToScreen

`worldToScreen(position)`

Converts `position` from world space to screen space. Screen coordinates are given in pixels. (0,0) is the top-left of the screen and ([Screen.width](/engine/screen#width),[Screen.height](/engine/screen#height)) is the bottom-right.

*Available since:* Open Surge 0.5.1

*Arguments*

* `position`: [Vector2](/engine/vector2) object. The position to be converted.

*Returns*

Returns a [Vector2](/engine/vector2) object corresponding to the converted coordinates.

#### screenToWorld

`screenToWorld(position)`

Converts `position` from screen space to world space. Screen coordinates are given in pixels. (0,0) is the top-left of the screen and ([Screen.width](/engine/screen#width),[Screen.height](/engine/screen#height)) is the bottom-right.

*Available since:* Open Surge 0.5.1

*Arguments*

* `position`: [Vector2](/engine/vector2) object. The position to be converted.

*Returns*

Returns a [Vector2](/engine/vector2) object corresponding to the converted coordinates.