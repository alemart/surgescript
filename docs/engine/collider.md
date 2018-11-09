Collider
========

A collider, or collision object, is used to detect collisions. There are different types of colliders, each with a different shape. They are usually centered on the hot spot of sprites, but that can be changed by altering their *anchor*.

Colliders must be spawned as children of [entities](entity). To detect collisions, you may implement function `onCollision()` on the entity (see the example below), or use the colliders directly. Additionally, a single entity may have multiple colliders attached to it. This allows users to work with more complex shapes than simple primitives.

A collider is an abstract concept, and hence can't be spawned directly. Rather, you can spawn colliders of specific shapes, such as [CollisionBox](collisionbox) and [CollisionBall](collisionball). All Colliders share some functionalities (detailed in this page), but there are functionalities tied to specific shapes.

*Example*

```
using SurgeEngine.Actor;
using SurgeEngine.Player;
using SurgeEngine.Collisions.CollisionBall;

object "CollisionDoll" is "entity"
{
    actor = Actor("CollisionDoll");
    collider = CollisionBall(25); // ball with radius = 25px

    // The player has a built-in collider
    // Let's make it visible for debugging
    state "main"
    {
        player = Player.active;
        player.collider.visible = true;
        collider.visible = true;
    }

    // Detect collisions between a collider that is a child
    // of this object and any other collider in the game
    fun onCollision(otherCollider)
    {
        // A collision has occurred.
        Console.print("Collided with something");

        // Collided with a player?
        if(otherCollider.entity.hasTag("player")) {
            player = otherCollider.entity;
            Console.print("Touched " + player.name);
        }
    }
}
```


Properties
----------

#### entity

`entity`: object, read-only.

The [Entity](entity) associated with this collider.

#### visible

`visible`: boolean.

Useful for debugging. The default value is `false`.

Functions
---------

#### collidesWith

`collidesWith(collider)`

Checks if this collider is colliding with some other collider.

*Arguments*

* `collider`: [Collider](collider) object. The other collider.

*Returns*

Returns `true` if there is a collision (the colliders overlap), or `false` otherwise.

#### contains

`contains(x, y)`

Checks if the collider contains the point (`x`, `y`), given in world coordinates.

*Arguments*

* `x`: number. The x-coordinate of the point.
* `y`: number. The y-coordinate of the point.

*Returns*

Returns `true` if the point is inside the collider, or `false` otherwise.

#### setAnchor

`setAnchor(x, y)`

Defines the anchor of the collider to be (`x`, `y`), where these values are (usually) numbers between 0.0 and 1.0. Imagine a bounding box of the collider. Point (0.5, 0.5) is the default, representing its center. Point (0.0, 0.0) is the top-left and (1,0, 1.0), the bottom-right. The anchor of the collider will be aligned to the hot spot of the sprite of the entity.

*Arguments*

* `x`: number. Usually a value between 0.0 and 1.0.
* `y`: number. Usually a value between 0.0 and 1.0.
