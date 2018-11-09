CollisionBall
=============

A CollisionBall is a special type of [Collider](collider) that takes the shape of a ball. In 2D space, this is a circle with a specific radius. All functions and properties of [Collider](collider) apply to this.

*Example*

```
using SurgeEngine.Actor;
using SurgeEngine.Collisions.CollisionBall;

object "CollisionTest" is "entity"
{
    actor = Actor("CollisionTest");
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

`SurgeEngine.Collisions.CollisionBall(radius)`

Spawns a new CollisionBall with the specified radius, in pixels.

*Arguments*

* `radius`: number. The radius of the CollisionBall.

*Returns*

A new CollisionBall with the specified radius.

Read-Only Properties
--------------------

* `centerX`: number. The x-coordinate of the center of the CollisionBall, in world space.
* `centerY`: number. The y-coordinate of the center of the CollisionBall, in world space.

Read-Write Properties
---------------------

* `radius`: number. The radius of the CollisionBall.
