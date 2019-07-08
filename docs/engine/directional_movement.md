Directional Movement
====================

The Directional Movement [behavior](/engine/behavior) makes the associated [entity](/engine/entity) perform a directional movement on the 2D plane. It features:

* A direction [vector](/engine/vector2) that specifies the direction of the movement
* A movement speed, a scalar value given in pixels per second

You may control the direction of the movement using a vector or an angle given in degrees. Additionally, you may control the speed of the movement by changing the speed property directly.

Directional Movement is very versatile. You can use it to implement many types of baddies, flying objects, racing cars viewed from a top-down view, circular movements, and much more.

*Example*

```
//
// The example below shows how to make an entity move to the
// right of the screen at a rate of 60 pixels per second
//
using SurgeEngine.Actor;
using SurgeEngine.Vector2;
using SurgeEngine.Behavior.DirectionalMovement;

object "Simple Ball" is "entity"
{
    actor = Actor("Simple Ball");
    movement = DirectionalMovement();

    state "main"
    {
        movement.direction = Vector2.right;
        movement.speed = 60;
    }
}
```

Factory
-------

#### Behavior.DirectionalMovement

`DirectionalMovement()`

Spawns a DirectionalMovement.

*Returns*

A DirectionalMovement object.

Properties
----------

#### speed

`speed`: number.

The speed of the movement, in pixels per second.

#### direction

`direction`: [Vector2](/engine/vector2) object.

The direction of the movement.

#### angle

`angle`: number.

The counterclockwise angle of the direction vector, in degrees. 0 means right, 90 means up, etc.

*Example*

```
//
// In the example below, a circular movement is performed
//
using SurgeEngine.Actor;
using SurgeEngine.Behavior.DirectionalMovement;

object "Simple Ball" is "entity"
{
    actor = Actor("Simple Ball");
    movement = DirectionalMovement();
    angularSpeed = 360; // one full rotation per second

    state "main"
    {
        movement.speed = 192;
        movement.angle += angularSpeed * Time.delta;
    }
}
```
