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

object "MyLevelObject" is "entity"
{
    public transform = Transform();
    actor = Actor("MyLevelObject");

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

Only use this property if you need to read the position of the object or to set it to a fixed value. If you need to move the entity, use [move](#move) instead.

*Example*
```
using SurgeEngine.Actor;
using SurgeEngine.Player;
using SurgeEngine.Transform;

object "MyLevelObject" is "entity"
{
    public transform = Transform();
    actor = Actor("MyLevelObject");

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

`localPosition`: [Vector2](/engine/Vector2) object.

The position of the object relative to the parent object.

Only use this property if you need to read the position of the object or to set it to a fixed value. If you need to move the entity, use [move](#move) instead.

#### localAngle

`localAngle`: number.

The angle, in degrees, relative to the angle of the parent object.

Unless you need to read or to set the angle of the object to a pre-determined value, consider using [rotate](#rotate) instead.

#### localScale

`localScale`: [Vector2](/engine/vector2) object.

The local scale of the entity.

Functions
---------

#### move

`move(dx, dy)`

Moves the object by (`dx`, `dy`).

*Arguments*

* `dx`: number. The amount to move the object in the x-axis.
* `dy`: number. The amount to move the object in the y-axis.

*Example*
```
using SurgeEngine.Actor;
using SurgeEngine.Transform;

object "MyLevelObject" is "entity"
{
    public transform = Transform();
    actor = Actor("MyLevelObject");

    state "main"
    {
        // move the entity at a speed of
        // 50 pixels/second, to the right
        transform.move(50 * Time.delta, 0);

        // the command below would move it
        // at a speed of 30 pixels per FRAME
        //transform.move(30, 0); // don't do this

        // since the framerate may vary across
        // different systems, it's advisable
        // to multiply the value by Time.delta
        // to make the behavior consistent.
    }
}
```

#### translate

`translate(delta)`

Translates the object by a `delta` vector.

*Arguments*

* `delta`: [Vector2](/engine/vector2) object.

#### rotate

`rotate(deg)`

Rotates the object by `deg` degrees.

*Arguments*

* `deg`: number.

*Example*
```
using SurgeEngine.Actor;
using SurgeEngine.Transform;

object "MyLevelObject" is "entity"
{
    public transform = Transform();
    actor = Actor("MyLevelObject");

    state "main"
    {
        // rotate the entity at a speed of
        // 45 degrees per second
        transform.rotate(45 * Time.delta);
    }
}
```

#### lookAt

`lookAt(transform)`

Use this function to make your object look at some point in space.

*Arguments*

* `transform`: Transform object.

*Example*
```
using SurgeEngine.Actor;
using SurgeEngine.Player;
using SurgeEngine.Transform;

object "MyLevelObject" is "entity"
{
    public transform = Transform();
    actor = Actor("MyLevelObject");

    state "main"
    {
        player = Player.active;
        transform.lookAt(player.transform);
    }
}
```
