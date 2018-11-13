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
    // spawns an Actor with the SurgeTest sprite
    actor = Actor("SurgeTest");

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

#### anim

`anim`: number.

A shortcut to `animation.id`: an integer corresponding to the animation number. Defaults to 0.

#### animation

`animation`: [Animation](animation) object, read-only.

Reference to the Animation object of the Actor.

#### alpha

`alpha`: number.

Opacity value, ranging from zero (0% opaque) to one (100% opaque). Defaults to 1.0.

#### entity

`entity`: object, read-only.

The entity associated with this component.

#### hflip

`hflip`: boolean.

Should the actor be flipped horizontally? Defaults to `false`.

#### vflip

`vflip`: boolean.

Should the actor be flipped vertically? Defaults to `false`.

#### visible

`visible`: boolean.

Should the actor be rendered? Defaults to `true`.

#### width

`width`: number, read-only.

The width of the actor.

#### height

`height`: number, read-only.

The height of the actor.

#### transform

`transform`: [Transform2D](../reference/transform2d) object.

An alias to the Transform of the entity.

#### zindex

`zindex`: number.

Objects with greater zindex are rendered in front of others. Defaults to 0.5.
