Sensor
======

A Sensor is used to detect collisions with bricks (obstacle, cloud). Due to performance optimizations, passable bricks (or bricks that are too far off camera) can't be sensed.

Factory
-------

#### Sensor

`SurgeEngine.Sensor(x, y, width, height)`

Spawns a new Sensor with the specified dimensions and having its top-left corner located at position (*x*, *y*) relative to the parent object. A Sensor is either a vertical or a horizontal bar that is 1-pixel thin. Both *width* and *height* must be positive integers, and at least one of them must be equal to 1.

*Arguments*

* `x`: number. The x-position of the top-left corner of the sensor, relative to the parent object.
* `y`: number. The y-position of the top-left corner of the sensor, relative to the parent object.
* `width`: number. The width of the sensor, in pixels. Must be a positive integer.
* `height`: number. The height of the sensor, in pixels. Must be a positive integer.

*Returns*

A Sensor with the specified parameters.

*Example*
```
using SurgeEngine.Actor;
using SurgeEngine.Sensor;

object "SensorToy" is "entity"
{
    actor = Actor("SensorToy");
    sensor = Sensor(10, -50, 1, 50);

    state "main"
    {
        actor.transform.xpos = 210;
        actor.transform.ypos = 150;
        sensor.visible = true;
        if(sensor.status != 0)
            Console.print("Got brick");
    }
}

object "Application"
{
    example = spawn("SensorExample");

    state "main"
    {
    }
}
```



Properties
----------

* `visible`: boolean. Should the sensor be rendered? Useful for debugging. Defaults to `false`.
* `status`: number, read-only. The type of the brick colliding with the sensor (0: no brick, 1: obstacle, 2: cloud).

Functions
---------

#### update

`update()`

The sensor status is updated automatically once per frame. Use this function to update it manually. This is useful if you modify the position of the parent object and need the updated sensor status in the same frame (for example; you're working in a loop).
