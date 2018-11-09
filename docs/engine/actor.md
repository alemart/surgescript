Actor
=====

The Actor component is used to associate a sprite to a target object. The target object is required to be tagged as an *"entity"*.

Factory
-------

#### Actor

`SurgeEngine.Actor(spriteName)`

Spawns a new Actor component with the given sprite name.

*Arguments*

* `spriteName`: string. The name of the sprite (defined in the *sprites/* folder).

*Returns*

An Actor component.

*Example*
```
using SurgeEngine.Actor;

object "SurgeTest" is "entity"
{
    // spawns an Actor with the SD_SURGE sprite
    actor = Actor("SD_SURGE");

    // position the actor
    state "main"
    {
        actor.transform.xpos = 210;
        actor.transform.ypos = 150;
    }
}

object "Application"
{
    surge = spawn("SurgeTest");

    state "main"
    {
    }
}
```



Properties
----------

* `anim`: number. An integer value corresponding to the animation number. Defaults to 0.
* `alpha`: number. Opacity value, ranging from zero (0% opaque) to one (100% opaque). Defaults to 1.0.
* `entity`: object. The entity associated with this component.
* `hflip`: boolean. Should the actor be flipped horizontally?
* `vflip`: boolean. Should the actor be flipped vertically?
* `visible`: boolean. Should the actor be rendered?
* `width`: number. The width of the actor.
* `height`: number. The height of the actor.
* `transform`: [Transform2D](../reference/transform2d) object. An alias to the Transform of the entity.
* `zindex`: number. The zindex of the actor. Objects with greater zindex are rendered in front of others. Defaults to 0.5.

Functions
---------

#### animFinished

`animFinished()`

Checks if the current animation has finished playing.

*Returns*

Returns `true` if the animation has finished playing.

#### animRepeats

`animRepeats()`

Checks if the currently playing animation repeats itself.

*Returns*

Returns `true` if the currently playing animation repeats itself.

#### animFPS

`animFPS()`

The frames-per-second (FPS) rate of the current animation.

*Returns*

Returns the FPS rate of the current animation.
