Sensor
======

A Sensor is used to detect collisions with bricks (obstacle, cloud). Due to performance optimizations, passable bricks (or bricks that are too far off camera) can't be sensed.

*Example*
```
using SurgeEngine.Actor;
using SurgeEngine.Collisions.Sensor;

object "SensorToy" is "entity"
{
    actor = Actor("SensorToy");
    sensor = Sensor(10, -50, 1, 50);

    state "main"
    {
        if(sensor.status != null)
            Console.print("Got a brick");
    }

    fun constructor()
    {
        sensor.visible = true;
    }
}
```



Factory
-------

#### Sensor

`Collisions.Sensor(x, y, width, height)`

Spawns a new Sensor with the specified dimensions and having its top-left corner located at position (*x*, *y*) relative to the parent object. A Sensor is either a vertical or a horizontal bar that is 1-pixel thin. Both *width* and *height* must be positive integers, and at least one of them must be equal to 1.

*Arguments*

* `x`: number. The x-position of the top-left corner of the sensor, relative to the parent object.
* `y`: number. The y-position of the top-left corner of the sensor, relative to the parent object.
* `width`: number. The width of the sensor, in pixels. Must be a positive integer.
* `height`: number. The height of the sensor, in pixels. Must be a positive integer.

*Returns*

A Sensor with the specified parameters.


Properties
----------

#### visible

`visible`: boolean.

Should the sensor be rendered? Useful for debugging. Defaults to `false`.

#### status

`status`: string | null, read-only.

The type of the brick colliding with the sensor (either *"solid"* or *"cloud"*). If the sensor isn't colliding with a brick, its status will be `null`.
