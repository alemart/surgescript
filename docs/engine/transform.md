Transform
=========

Transform encapsulates a 2D Transform. It comes with functions for translating, rotating and scaling things - and it's very useful for games! Whenever you spawn a Transform on your object, you gain power to command its position, rotation and scale.

The Transform component can only be spawned on [entities](/engine/entity). If an entity with a Transform component has any children, the transform of the children will be affected by the transform of their parent. Example: if you move an object, its children will be moved by the same amount.

Factory
-------

#### Transform

`Transform()`

Spawns a Transform on the object. The object is required to be an [entity](/engine/entity).

*Returns*

A Transform object.

*Example*
```
using SurgeEngine.Actor;
using SurgeEngine.Transform;

object "My Level Object" is "entity"
{
    public transform = Transform();
    actor = Actor("My Level Object");

    state "main"
    {
        // prints the position of this entity
        Console.print(transform.position);
    }
}
```

Properties
----------

#### position

`position`: [Vector2](/engine/vector2) object.

The position of the object in world coordinates.

Only use this property if you need to read the position of the object or to set it to a fixed value. If you need to move the entity, use [translateBy](#translateBy) instead.

*Example*
```
using SurgeEngine.Actor;
using SurgeEngine.Player;
using SurgeEngine.Transform;

object "My Level Object" is "entity"
{
    public transform = Transform();
    actor = Actor("My Level Object");

    state "main"
    {
        // this will show the distance between
        // this entity and the active player
        player = Player.active;
        Console.print(distanceTo(player));
    }

    fun distanceTo(obj)
    {
        return transform.position.distanceTo(obj.transform.position);
    }
}
```

#### angle

`angle`: number.

The angle, in degrees, specifying the rotation of the object.

Unless you need to read or to set the angle of the object to a pre-determined value, consider using [rotate](#rotate) instead.

#### localPosition

`localPosition`: [Vector2](/engine/vector2) object.

The position of the object relative to the parent object.

Only use this property if you need to read the position of the object or to set it to a fixed value. If you need to move the entity, use [translateBy](#translateBy) instead.

#### localAngle

`localAngle`: number.

The angle, in degrees, relative to the angle of the parent object.

Unless you need to read or to set the angle of the object to a pre-determined value, consider using [rotate](#rotate) instead.

#### localScale

`localScale`: [Vector2](/engine/vector2) object.

The local scale of the object. A value of 1.0 in both axes means the default size. A value of 2.0, twice the default size, and so on.

#### lossyScale

`lossyScale`: [Vector2](/engine/vector2) object, read-only.

An approximation of the world scale of the object. It's not very accurate (the parent transform may be rotated and scaled), but it's useful.

*Available since:* Open Surge 0.5.1

#### right

`right`: [Vector2](/engine/vector2) object, read-only.

The right vector of the transform in world space. Unlike [Vector2.right](/engine/vector2#vector2right), this property considers the rotation of the transform.

*Available since:* Open Surge 0.5.1

#### up

`up`: [Vector2](/engine/vector2) object, read-only.

The up vector of the transform in world space. Unlike [Vector2.up](/engine/vector2#vector2up), this property considers the rotation of the transform.

*Available since:* Open Surge 0.5.1

Functions
---------

#### translateBy

`translateBy(dx, dy)`

Translates (moves) the object by (`dx`, `dy`).

*Available since:* Open Surge 0.5.1. See the note below.

*Arguments*

* `dx`: number. Specifies how much to move the object in the x-axis.
* `dy`: number. Specifies how much to move the object in the y-axis.

*Example*
```
using SurgeEngine.Actor;
using SurgeEngine.Transform;

object "My Level Object" is "entity"
{
    public transform = Transform();
    actor = Actor("My Level Object");

    state "main"
    {
        // move the entity at a speed of
        // 50 pixels/second, to the right
        transform.translateBy(50 * Time.delta, 0);

        // the command below would move it
        // at a speed of 30 pixels per FRAME
        //transform.translateBy(30, 0); // don't do this

        // since the framerate may vary across
        // different systems, it's advisable
        // to multiply the value by Time.delta
        // to make the behavior consistent.
    }
}
```

*Note:* in versions prior to 0.5.1, this function was called `move`.

#### translate

`translate(delta)`

Translates the object by a `delta` vector.

*Arguments*

* `delta`: [Vector2](/engine/vector2) object.

#### rotate

`rotate(deg)`

Rotates the object by `deg` degrees.

*Arguments*

* `deg`: number. An angle in degrees.

*Example*
```
using SurgeEngine.Actor;
using SurgeEngine.Transform;

object "My Level Object" is "entity"
{
    public transform = Transform();
    actor = Actor("My Level Object");

    state "main"
    {
        // rotate the entity at a speed of
        // 45 degrees per second
        transform.rotate(45 * Time.delta);
    }
}
```

#### scaleBy

`scaleBy(sx, sy)`

Scales the object by horizontal and vertical factors `sx` and `sy`, respectively.

*Available since:* Open Surge 0.6.1.

*Arguments*

* `sx`: number. Horizontal scale factor. Value `1.0` means no change.
* `sy`: number. Vertical scale factor. Value `1.0` means no change.

#### scale

`scale(resize)`

Scales the object by a `resize` vector.

*Available since:* Open Surge 0.6.1.

*Arguments*

* `resize`: [Vector2](/engine/vector2) object. `Vector2(1,1)` means no change.

#### lookAt

`lookAt(position)`

Rotates the transform so that its [right](#right) vector points at the given position. Use this function to make your object look at some point in world space.

*Available since:* Open Surge 0.5.1. See the note below.

*Arguments*

* `position`: [Vector2](/engine/vector2) object. A point in world space.

*Example*
```
using SurgeEngine.Actor;
using SurgeEngine.Player;
using SurgeEngine.Transform;

object "My Level Object" is "entity"
{
    public transform = Transform();
    actor = Actor("My Level Object");

    state "main"
    {
        player = Player.active;
        transform.lookAt(player.transform.position);
    }
}
```

*Note:* in versions prior to 0.5.1, this function accepted a Transform parameter.
