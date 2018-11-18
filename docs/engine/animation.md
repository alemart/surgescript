Animation
=========

Animation objects are used to gather data about specific animations. Although you can't spawn Animation objects directly, you can access them via other objects such as [Actor](actor) and [Player](player).

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

#### fps

`fps`: number, read-only.

Frames per second of the current animation.

#### finished

`finished`: boolean, read-only.

Will be true if the current animation has finished playing.

#### repeats

`repeats`: boolean, read-only.

Does the current animation repeat itself?

#### frame

`frame`: number, read-only.

The current frame of the animation: an integer between `0` and `frameCount - 1`, inclusive.

#### frameCount

`frameCount`: number, read-only.

The number of frames of the current animation.

#### hotspotX

`hotspotX`: number, read-only.

The x-position of the hot spot of the current animation.

#### hotspotY

`hotspotY`: number, read-only.

The y-position of the hot spot of the current animation.
