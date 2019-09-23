Circular Movement
=================

The Circular Movement [behavior](/engine/behavior) makes the associated [entity](/engine/entity) perform a circular orbit on the 2D plane. Its basic parameters are:

* A radius, given in pixels
* A movement rate, given in cycles per second

Other parameters include: a movement scale in both X and Y axes (making it an elliptic orbit), a flag telling whether the movement should be clockwise or not, and so on.

*Example*

```
//
// The example below shows how to make an entity move along
// a circle of 128 pixels of radius at a rate of 0.25 cycles
// per second (i.e., it takes one second to complete 25% of
// a cycle, or 4 seconds to complete a cycle)
//
using SurgeEngine.Actor;
using SurgeEngine.Vector2;
using SurgeEngine.Behaviors.CircularMovement;

object "Simple Ball" is "entity"
{
    actor = Actor("Simple Ball");
    movement = CircularMovement();

    state "main"
    {
        movement.radius = 128;
        movement.rate = 0.25;
        //movement.scale = Vector2.up; // uncomment to move along the y-axis only
    }
}
```

Factory
-------

#### Behaviors.CircularMovement

`CircularMovement()`

Spawns a CircularMovement.

*Returns*

A CircularMovement object.

Properties
----------

#### radius

`radius`: number.

The radius of the movement, in pixels.

#### rate

`rate`: number.

The rate of the movement, given in cycles per second.

#### clockwise

`clockwise`: boolean.

Indicates whether the movement is clockwise (`true`) or counterclockwise (`false`). Defaults to `false`.

#### scale

`scale`: [Vector2](/engine/vector2) object.

Specifies the scale of the movement in both X and Y axes. It is used to distort the circle. `Vector2(1, 1)` means no distortion (default).

#### center

`center`: [Vector2](/engine/vector2) object | null.

If not null, forces the center of the movement to be at a particular position in world space. Defaults to `null`.

#### phaseOffset

`phaseOffset`: number.

A value in degrees that offsets the current [phase](#phase). Defaults to zero (180 means opposite phase relative to zero).

#### phase

`phase`: number, read-only.

A value in degrees that indicates the current phase of the movement.
