Vector2
=======

Vector2 is a vector in 2D space. A vector is a mathematical object that has direction and length, usually represented by an arrow. A Vector2 can also be represented by a pair of *(x,y)* coordinates.

In SurgeScript, vectors are immutable. Once created, you can't change their coordinates directly. In order to bring about change (say, in the position of an [entity](/engine/entity)), you need to create a new vector.

Vector2 objects have been designed in such a way that their functions make them chainable. You can compose different vector operations using english-based terms, as in the example below:

*Example*

```
using SurgeEngine.Player;
using SurgeEngine.Vector2;
...
player = Player.active;
...
l = Vector2(6, 8).scaledBy(0.5).length; // number five
u = Vector2.right.rotatedBy(45).scaledBy(2); // a vector
v = player.transform.position.translatedBy(5,0); // a vector
...
```

Factory
-------

#### Vector2

`Vector2(x, y)`

Spawns a 2D vector with the given coordinates.

*Arguments*

* `x`: number. The x-coordinate of the vector.
* `y`: number. The y-coordinate of the vector.

*Returns*

A Vector2 object.

*Example*
```
using SurgeEngine.Vector2;
using SurgeEngine.Player;

// This entity will lock the player on position (100, 100)
object "PlayerLocker" is "entity", "awake"
{
    pos = Vector2(100, 100);

    state "main"
    {
        player = Player.active;
        player.transform.position = pos;
    }
}
```

#### Vector2.up

`Vector2.up`

The unit up vector.

*Returns*

Returns the unit up vector.

#### Vector2.right

`Vector2.right`

The unit right vector.

*Returns*

Returns the unit right vector.

#### Vector2.down

`Vector2.down`

The unit down vector.

*Returns*

Returns the unit down vector.

#### Vector2.left

`Vector2.left`

The unit left vector.

*Returns*

Returns the unit left vector.

#### Vector2.zero

`Vector2.zero`

The (0,0) vector.

*Returns*

Returns the (0,0) vector.


Properties
----------

#### x

`x`: number, read-only.

The x-coordinate of the vector.

#### y

`y`: number, read-only.

The y-coordinate of the vector.

#### length

`length`: number, read-only.

The length of the vector.

#### angle

`angle`: number, read-only.

The angle, in degrees, between the vector and the positive x-axis (as in polar coordinates).

Functions
---------

#### plus

`plus(v)`

Returns a Vector2 corresponding to the addition between `this` and `v`.

*Arguments*

* `v`: Vector2 object.

*Returns*

Returns a Vector2 object corresponding to the result of the operation.

*Example*

```
a = Vector2(3, 1);
b = Vector2(2, 1);
c = a.plus(b); // c = (5,2)
```

#### minus

`minus(v)`

Returns a Vector2 corresponding to the subtraction between `this` and `v`.

*Arguments*

* `v`: Vector2 object.

*Returns*

Returns a Vector2 object corresponding to the result of the operation.

*Example*

```
v = Vector2(5, 5);
zero = v.minus(v); // zero = (0,0)
```

#### dot

`dot(v)`

Returns the dot product between `this` and `v`.

*Arguments*

* `v`: Vector2 object.

*Returns*

Returns a number: the dot product between `this` and `v`.

#### normalized

`normalized()`

Returns a normalized copy of `this`: the new vector will have length one.

*Returns*

Returns a Vector2 object corresponding to the result of the operation.

#### directionTo

`directionTo(v)`

Returns a unit vector pointing to `v` (from `this`).

*Arguments*

* `v`: Vector2 object.

*Returns*

Returns a Vector2 of length one corresponding to the result of the operation.

#### distanceTo

`distanceTo(v)`

Considering `this` and `v` as points in space, this function returns the distance between them.

*Arguments*

* `v`: Vector2 object.

*Returns*

Returns a number corresponding to the specified distance.

*Example*
```
using SurgeEngine.Transform;
using SurgeEngine.Player;

// This entity will show the distance between
// itself and the active player
object "DistanceDebugger" is "entity", "awake"
{
    transform = Transform();

    state "main"
    {
        player = Player.active;
        playerpos = player.transform.position;
        distance = transform.position.distanceTo(playerpos);
        Console.print(distance);
    }
}
```

#### translatedBy

`translatedBy(dx,dy)`

Returns a copy of `this` translated by `(dx,dy)`.

*Arguments*

* `dx`: number. The offset to be added to the x-coordinate.
* `dy`: number. The offset to be added to the y-coordinate.

*Returns*

Returns a Vector2 object corresponding to the result of the operation.

*Example*

```
using SurgeEngine.Vector2;
...
one = Vector2(1,1);
v = one.translatedBy(4,5); // (5,6)
```

#### rotatedBy

`rotatedBy(deg)`

Returns a copy of `this` rotated by `deg` degrees.

*Arguments*

* `deg`: number. The amount of degrees used on the rotation.

*Returns*

Returns a Vector2 object corresponding to the result of the operation.

*Example*

```
using SurgeEngine.Vector2;

...

// A unit vector with an angle of 45 degrees
// (relative to the positive x-axis)
v = Vector2.right.rotatedBy(45);
```

#### scaledBy

`scaledBy(s)`

Returns a copy of `this` scaled by `s`. The length of the resulting vector will be the length of `this` multiplied by `s`.

*Arguments*

* `s`: number. The multiplier.

*Returns*

Returns a Vector2 object corresponding to the result of the operation.

*Example*

```
using SurgeEngine.Vector2;
...
one = Vector2(1,1);
two = one.scaledBy(2); // (2,2)
half = one.scaledBy(0.5); // (0.5,0.5)
```

#### projectedOn

`projectedOn(v)`

Returns a copy of `this` projected on nonzero vector `v`.

*Arguments*

* `v`: Vector2 object.

*Returns*

Returns a Vector2 object corresponding to the result of the operation.

#### toString

`toString()`

Converts the vector to a string.

*Returns*

Returns a string containing the coordinates of `this`.

*Example*

```
using SurgeEngine.Vector2;
...
one = Vector2(1,1);
Console.print(one); // using toString() implicitly
```
