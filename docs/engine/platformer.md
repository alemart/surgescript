Platformer
==========

The Platformer [behavior](/engine/behavior) makes the associated [entity](/engine/entity) perform a simple platform movement (with gravity, etc). Used mostly by NPCs and baddies, this behavior is supposed to be lightweight for the CPU and is not designed to support 360°-physics.

**Important:** for best results, the hot spot of the entity should be placed on its feet.

*Example*

```cs
//
// In the example below, we create a dummy that
// walks around the level (left and right)
//
using SurgeEngine.Actor;
using SurgeEngine.Behaviors.Enemy;
using SurgeEngine.Behaviors.Platformer;

object "My Baddie" is "entity", "enemy"
{
    actor = Actor("My Baddie"); // give it graphics
    enemy = Enemy(); // make it behave like a baddie
    platformer = Platformer(); // give it a platform movement

    state "main"
    {
        platformer.speed = 60; // 60 pixels per second
        platformer.walk(); // make it walk
    }
}
```

Factory
-------

#### Behaviors.Platformer

`Platformer()`

Spawns a Platformer.

*Returns*

The behavior object.

Properties
----------

#### speed

`speed`: number.

Walking speed, in pixels per second.

#### jumpSpeed

`jumpSpeed`: number.

Jump speed, in pixels per second. The higher the value, the more intense the jump.

#### gravityMultiplier

`gravityMultiplier`: number.

A multiplier used to modify how the Platformer is affected by gravity. When set to 1.0, the Platformer will subject to the [default acceleration of gravity](/engine/level#gravity). When set to 2.0 (0.5), the Platformer will be subject to twice (half) the default gravity, and so on. Zero means no gravity. Defaults to 1.0.

*Available since:* Open Surge 0.6.0

#### direction

`direction`: number, read-only.

Direction will be +1 if the platformer is facing right or -1 is it's facing left.

#### walking

`walking`: boolean, read-only.

Is the platformer walking?

#### walkingLeft

`walkingLeft`: boolean, read-only.

Is the platformer walking to the left?

#### walkingRight

`walkingRight`: boolean, read-only.

Is the platformer walking to the right?

#### midair

`midair`: boolean, read-only.

Is the platformer midair?

#### falling

`falling`: boolean, read-only.

Is the platformer falling down?

#### wall

`wall`: boolean, read-only.

Is the platformer touching a wall?

#### leftWall

`leftWall`: boolean, read-only.

Is the platformer touching a wall on its left side?

#### rightWall

`rightWall`: boolean, read-only.

Is the platformer touching a wall on its right side?

#### leftLedge

`leftLedge`: boolean, read-only.

Is the platformer standing on a ledge located on its left side?

#### rightLedge

`rightLedge`: boolean, read-only.

Is the platformer standing on a ledge located on its right side?

Functions
---------

#### walk

`walk()`

Enables automatic walking. The platformer will walk left and right automatically.

*Returns*

Returns the platformer itself.

#### walkLeft

`walkLeft()`

Makes the platformer walk to the left.

*Returns*

Returns the platformer itself.

#### walkRight

`walkRight()`

Makes the platformer walk to the right.

*Returns*

Returns the platformer itself.

#### stop

`stop()`

Makes the platformer stop walking.

*Returns*

Returns the platformer itself.

#### jump

`jump()`

Makes the platformer jump. It will only jump if it's touching the ground.

*Returns*

Returns the platformer itself.

*Example*

```cs
//
// In the example below, we create a jumping
// dummy that walks left and right
//
using SurgeEngine.Actor;
using SurgeEngine.Behaviors.Enemy;
using SurgeEngine.Behaviors.Platformer;

object "My Jumping Baddie" is "entity", "enemy"
{
    actor = Actor("My Jumping Baddie"); // handles the graphics
    enemy = Enemy(); // make it behave like a baddie
    platformer = Platformer().walk(); // make it walk

    state "main"
    {
        platformer.speed = 60; // 60 pixels per second
        if(timeout(3.0)) // jump every 3 seconds
            state = "jump";
    }

    state "jump"
    {
        platformer.jump();
        state = "main";
    }
}
```

#### forceJump

`forceJump(speed)`

Makes the platformer jump, regardless if it's touching the ground or not.

*Arguments*

* `speed`: number. Jump speed in pixels per second.

*Returns*

Returns the platformer itself.

#### setSensorBox

`setSensorBox(width, height)`

The platformer spawns invisible [sensors](/engine/sensor) to detect collisions. Together, these sensors are thought to form a box which size is related to the size of the individual sensors. This function sets the size of this box. That size should be compatible with the size of the sprite and a size too small may break the platformer. The size of the sensor box is computed automatically, so normally you don't need to modify it.

*Arguments*

* `width`: number. The width of the box, in pixels.
* `height`: number. The height of the box, in pixels.

*Returns*

Returns the platformer itself.

#### showSensors

`showSensors()`

Displays the internal sensors of the platformer, which are invisible by default. For debugging only.

*Available since:* Open Surge 0.5.2

*Returns*

Returns the platformer itself.

*Example*

```cs
using SurgeEngine.Actor;
using SurgeEngine.Behaviors.Platformer;

object "Testing Stuff" is "entity"
{
    actor = Actor("Testing Stuff");
    platformer = Platformer().showSensors();
    // ...
}
```

#### hideSensors

`hideSensors()`

Hides the internal sensors of the platformer. This is the default.

*Available since:* Open Surge 0.5.2

*Returns*

Returns the platformer itself.
