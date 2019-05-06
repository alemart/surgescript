Animation
=========

Animation objects are used to gather data about specific animations. Although you can't spawn Animation objects directly, you can access them via other objects such as [Actor](../actor) and [Player](../player).

*Example*

```
using SurgeEngine.Actor;

object "MyExplosion" is "entity", "disposable", "private"
{
    actor = Actor("MyExplosion");

    state "main"
    {
        if(actor.animation.finished)
            destroy();
    }
}
```

Properties
----------

#### id

`id`: number.

The number of the current animation, defined in a .spr file.

#### sprite

`sprite`: string, read-only.

The name of the sprite, defined in a .spr file.

#### finished

`finished`: boolean, read-only.

Will be true if the current animation has finished playing.

#### hotspot

`hotspot`: [Vector2](vector2) object, read-only.

The position of the hot spot of the current animation.

#### fps

`fps`: number, read-only.

Frames per second of the current animation.

#### repeats

`repeats`: boolean, read-only.

Does the current animation repeat itself?

#### frame

`frame`: number, read-only.

The current frame of the animation: an integer between `0` and `frameCount - 1`, inclusive.

#### frameCount

`frameCount`: number, read-only.

The number of frames of the current animation.

#### speedFactor

`speedFactor`: number.

While the [FPS](#fps) rate controls the speed of the animation, the speed factor gives you an additional degree of control. This is a multiplier that defaults to 1.0, meaning that the animation will run using its normal speed. If it's set to 2.0, it will run using twice that speed. A value of 0.5 means half the speed, and so on.
