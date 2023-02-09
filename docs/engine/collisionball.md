CollisionBall
=============

A CollisionBall is a special type of [Collider](/engine/collider) that takes the shape of a ball. In 2D space, this is a circle with a specific radius. All functions and properties of [Collider](/engine/collider) apply to this.

*Example*

```cs
using SurgeEngine.Actor;
using SurgeEngine.Collisions.CollisionBall;

object "CollisionDoll" is "entity"
{
    actor = Actor("CollisionDoll");
    collider = CollisionBall(25); // radius = 25px

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

#### Collisions.CollisionBall

`Collisions.CollisionBall(radius)`

Spawns a new CollisionBall with the specified radius, in pixels.

*Arguments*

* `radius`: number. The radius of the CollisionBall.

*Returns*

A new CollisionBall with the specified radius.

Properties
----------

#### center

`center`: [Vector2](/engine/vector2), read-only.

The center of the CollisionBall, in world space.

#### radius

`radius`: number.

The radius of the CollisionBall, in pixels.
