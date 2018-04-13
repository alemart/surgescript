Time
====

Time utilities. This object is available simply by typing `Time`.

Example
-------

```
// This object will be moved at a rate of 20 pixels per second
object "MoveTest"
{
    transform = spawn("Transform2D");
    // ...

    state "main"
    {
        // CORRECT: this adds 20 pixels to transform.xpos at every second
        transform.xpos += 20 * Time.delta;

        // INCORRECT: this adds 30 pixels to transform.ypos at every FRAME
        //transform.ypos += 30;

        // since the framerate may vary across different systems, it's advisable
        // to multiply the value by Time.delta to make the behavior consistent.

        // ...
    }
}
```

Properties
----------

* `time`: number. The number of seconds since the application was started at the beginning of this frame.
* `delta`: number. The time, in second, taken to complete the last frame of the application. Use this value to make your application behave consistently regardless of the frame rate.
* `tickCount`: number. The number of seconds since the application was started at the moment this property is evaluated\*.

> **Note:**
>
> \* Since `Time.tickCount` may return different values during the same frame of your application, it's generally advisable to use `Time.time` instead.