Time
====

Time utilities. This object is available simply by typing `Time`.

Properties
----------

* `time`: number. The number of seconds since the application was started at the beginning of this frame.
* `delta`: number. The time, in second, taken to complete the last frame of the application. Use this value to make your application behave consistently regardless of the frame rate.
* `tickCount`: number. The number of seconds since the application was started at the moment this property is evaluated\*.

> **Note:**
>
> \* Since `Time.tickCount` may return different values during the same frame of your application, it's generally advisable to use `Time.time` instead.