Time
====

Time utilities. This object is available simply by typing `Time`.

Properties
----------

#### time

`time`: number, read-only.

Elapsed time, in seconds, since the application was started and measured at the beginning of the current frame.

#### delta

`delta`: number, read-only.

The time, in seconds, taken to complete the last frame of the application. Use this value to make your application behave consistently regardless of the frame rate.

#### now

`now`: number, read-only.

Elapsed time, in seconds, since the application was started and measured at the moment this property is evaluated.

*Available since:* SurgeScript 0.5.4

!!! tip

    It's generally advisable to use [Time.time](#time) instead, because [Time.now](#now) may return different values in the same frame of your application.