CollisionBox
============

A CollisionBox is a special type of [Collider](collider) that takes the shape of a box. In 2D space, this is a rectangle with a specific width and height. All functions and properties of [Collider](collider) apply to this.

*Example*

```
using SurgeEngine.Actor;
using SurgeEngine.Collisions.CollisionBox;

object "CollisionDoll" is "entity"
{
    actor = Actor("CollisionDoll");
    collider = CollisionBox(32,64); // width = 32px, height = 64px

    state "main"
    {
        collider.visible = true; // useful for debugging
    }

    fun onCollision(otherCollider)
    {
        Console.print("A collision has occurred.");
    }
}
```

Factory
-------

#### Collisions.CollisionBox

`SurgeEngine.Collisions.CollisionBox(width, height)`

Spawns a new CollisionBox with the specified dimensions, in pixels.

*Arguments*

* `width`: number. The width of the CollisionBox.
* `height`: number. The height of the CollisionBox.

*Returns*

A new CollisionBox with the specified dimensions.

Properties
----------

#### top

`top`: number, read-only.

The y-coordinate of top border of the CollisionBox, in world space.

#### right

`right`: number, read-only.

The x-coordinate of right border of the CollisionBox, in world space.

#### bottom

`bottom`: number, read-only.

The y-coordinate of bottom border of the CollisionBox, in world space.

#### left

`left`: number, read-only.

The x-coordinate of left border of the CollisionBox, in world space.

#### width

`width`: number.

The width of the CollisionBox, in pixels.

#### height

`height`: number.

The height of the CollisionBox, in pixels.
