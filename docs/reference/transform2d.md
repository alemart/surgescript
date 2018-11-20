Transform2D
===========

Transform2D encapsulates a 2D Transform. Transform2D comes with functions for translating, rotating and scaling things - and it's very useful for games! Whenever you spawn a Transform2D component on your object, it gains position, rotation and scale in 2D space.

If an object with a Transform2D component has any children, the transform of the children will be affected by the transform of their parent. For example: if you move (translate) the parent object, the children will be moved as well.

Properties
----------

#### xpos

`xpos`: number.

The x-position of the object in local coordinates¹.

#### ypos

`ypos`: number.

The y-position of the object in local coordinates¹.

#### angle

`angle`: number.

The angle specifying the local rotation of the object².

#### scaleX

`scaleX`: number.

Local horizontal scale (defaults to 1.0). *Since SurgeScript 0.5.3*.

#### scaleY

`scaleY`: number.

Local vertical scale (defaults to 1.0). *Since SurgeScript 0.5.3*.

#### worldX

`worldX`: number.

The x-position of the object in world coordinates.

#### worldY

`worldY`: number.

The y-position of the object in world coordinates.

#### worldAngle

`worldAngle`: number.

The rotation angle² of the object in world coordinates.

> **Notes:**
>
> ¹ Local coordinates mean: coordinates relative to the parent object.
>
> ² Rotation angles should be given in degrees.

Functions
---------

#### translate

`translate(xpos, ypos)`

Moves the object by (`xpos`, `ypos`).

*Arguments*

* `xpos`: number.
* `ypos`: number.

#### rotate

`rotate(degrees)`

Rotates the object by `degrees`.

*Arguments*

* `degrees`: number.

#### lookAt

`lookAt(transform)`

Use this function to make your object look at some point in space.

*Arguments*

* `transform`: Transform2D object.

#### distanceTo

`distanceTo(transform)`

Compute the distance between two points in space.

*Arguments*

* `transform`: Transform2D object.

*Returns*

A number representing the distance, in world coordinates, between the position stored by `this` and `transform`.
