Animation
=========

Animation objects are used to gather data about specific animations. Although you can't spawn Animation objects directly, you can access them via other objects such as [Actor](/engine/actor) and [Player](/engine/player).

*Example*

```cs
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

The number of the animation, defined in a .spr file.

#### sprite

`sprite`: string, read-only.

The name of the sprite, defined in a .spr file.

#### exists

`exists`: boolean, read-only.

Will be `true` if the animation exists, i.e., if its sprite and its animation number have been defined in a .spr file.

*Available since:* Open Surge 0.5.1

#### finished

`finished`: boolean, read-only.

Will be `true` if the animation has finished playing.

#### anchor

`anchor`: [Vector2](/engine/vector2) object, read-only.

The hot spot of the animation normalized to [0,1] x [0,1].

*Available since:* Open Surge 0.6.0

#### hotSpot

`hotSpot`: [Vector2](/engine/vector2) object, read-only.

The hot spot of the animation. Coordinates are given in pixels.

*Note:* prior to Open Surge 0.6.0, this property was called `hotspot`.

#### actionSpot

`actionSpot`: [Vector2](/engine/vector2) object, read-only.

The action spot of the animation. Coordinates are given in pixels. If the sprite is flipped, the action spot is automatically flipped relative to the [hot spot](#hotspot) of the animation.

*Available since:* Open Surge 0.6.0

#### actionOffset

`actionOffset`: [Vector2](/engine/vector2) object, read-only.

When this vector is added to the position of the sprite, you'll get the position of the [action spot](#actionspot). This is suitable to be used with [transform.localPosition](/engine/transform#localposition).

*Available since:* Open Surge 0.6.0

#### repeats

`repeats`: boolean, read-only.

Does the animation repeat itself?

#### fps

`fps`: number, read-only.

Frames per second of the animation.

#### frameCount

`frameCount`: number, read-only.

The number of frames of the animation.

#### duration

`duration`: number, read-only.

The duration of the animation, in seconds.

*Available since:* Open Surge 0.6.1

#### frame

`frame`: number.

The current frame of the animation: an integer between `0` and `frameCount - 1`, inclusive.

#### speedFactor

`speedFactor`: number.

While the [FPS](#fps) rate controls the speed of the animation, the speed factor gives you an additional degree of control. This is a multiplier that defaults to 1.0, meaning that the animation will run using its normal speed. If it's set to 2.0, it will run using twice that speed. A value of 0.5 means half the speed, and so on.

#### sync

`sync`: boolean.

Is the animation is synchronized? A synchronized animation is a repeating animation that displays the same frame across multiple sprites. Defaults to `false`.

Functions
---------

#### prop

`prop(propertyName)`

Read the user-defined custom property named `propertyName` defined in a *custom_properties* block of the [sprite](#sprite) of this animation (.spr file). If the property exists, this function will return a string, a number, a boolean, or an [Array](/reference/array) of these, depending on the property. If the property does not exist, this function will return `null`.

*Available since:* Open Surge 0.6.1

*Arguments*

* `propertyName`: string. The name of a custom property.

*Returns*

Returns a string, a number, a boolean, an array of these primitive types with at least two elements, or `null`.

*Example*

```cs
/*

Example of a custom_properties block:

// .spr file
sprite "My Test Sprite"
{
    // ...

    custom_properties
    {
        number_of_layers        8
        want_awesomeness        true
        font_name               "GoodNeighbors"
        position                100 200
    }
}

*/

// It's a good idea to cast the value of the property to the expected type, so
// that your script will work reliably regardless of what is in the .spr file!

animation = actor.animation;

numberOfLayers = Number(animation.prop("number_of_layers")); // 8
wantAwesomeness = Boolean(animation.prop("want_awesomeness")); // true
fontName = String(animation.prop("font_name") || "Default Font Name"); // "GoodNeighbors"
foobar = String(animation.prop("foobar") || "undefined"); // "undefined"

position = animation.prop("position");
if(typeof position == "object") {
    xpos = Number(position[0]); // 100
    ypos = Number(position[1]); // 200
}
else {
    xpos = 0;
    ypos = 0;
}
```
