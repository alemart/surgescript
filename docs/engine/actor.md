Actor
=====

The Actor component is used to associate a sprite to a target object. The target object is required to be an [entity](/engine/entity). It's recommended to use only one actor per entity.

Factory
-------

#### Actor

`Actor(sprite)`

Spawns a new Actor component with the given sprite name.

*Arguments*

* `sprite`: string. The name of the sprite (defined in the *sprites/* folder).

*Returns*

An Actor component.

*Example*
```
using SurgeEngine.Actor;

object "SurgeTest" is "entity"
{
    // spawns an Actor with the SurgeTest sprite
    actor = Actor("SurgeTest");

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

`animation`: [Animation](/engine/animation) object, read-only.

Reference to the Animation object of the Actor.

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

#### alpha

`alpha`: number.

Opacity value, ranging from zero (0% opaque) to one (100% opaque). Defaults to 1.0.

#### offset

`offset`: [Vector2](/engine/vector2) object.

A *(x,y)* offset relative to the parent object. Defaults to zero.

#### hotspot

`hotspot`: [Vector2](/engine/vector2) object.

A shortcut to `animation.hotspot`: the hot spot of the current animation. Coordinates are given in pixels. In versions prior to 0.6.0, you may get the hot spot from the [Animation](/engine/animation) object.

*Available since:* Open Surge 0.6.0

#### anchor

`hotspot`: [Vector2](/engine/vector2) object.

A shortcut to `animation.anchor`: the hot spot of the current animation normalized to [0,1] x [0,1].

*Available since:* Open Surge 0.6.0

#### width

`width`: number, read-only.

The width of the actor.

#### height

`height`: number, read-only.

The height of the actor.

#### zindex

`zindex`: number.

Objects with greater zindex are rendered in front of others. Defaults to 0.5.
