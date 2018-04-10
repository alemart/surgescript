Transform2D
===========

Transform2D encapsulates a 2D Transform. Transform2D comes with neat functions for translating, rotating and scaling things - and it's very useful for games! Whenever you spawn a Transform2D component on your object, it gains a position and a rotation in 2D space.

If an object with a Transform2D component has any children, the transform of the children will be affected by the transform of their parent. For example: if you move the parent object, the children will be moved as well.

Properties
----------

* `xpos`: number, read-write. The x-position of the object in local coordinates¹.
* `ypos`: number, read-write. The y-position of the object in local coordinates¹.
* `angle`: number, read-write. The angle specifying the rotation of the object².
* `scaleX`: number, read-write. Scale multiplier in the x-axis - defaults to 1.0.
* `scaleY`: number, read-write. Scale multiplier in the y-axis - defaults to 1.0.
* `worldX`: number, read-write. The x-position of the object in world coordinates.
* `worldY`: number, read-write. The y-position of the object in world coordinates.
* `worldAngle`: number, read-write. The rotation angle² of the object in world coordinates.

> **Notes:**
>
> ¹ Local coordinates mean: relative to the parent.
>
> ² Rotation angles should be given in degrees.

Methods
-------

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

#### scale

`scale(x, y)`

Scales the transform by (`x`, `y`). Both parameters are multipliers that default to 1.0.

*Arguments*

* `x`: number.
* `y`: number.

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