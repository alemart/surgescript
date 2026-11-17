Timer
=====

The Timer component fires (emits a [signal](../tutorials/signals.md)) after a specified time. Unlike [timeout](../tutorials/expressions.md#timeout), the Timer component works independently of the current state of your objects.

*Example*

```cs
using SurgeScript.Timer;

object "Spaceship"
{
    timer = Timer()
            ..waitTime = 1.0
            ..repeat = true
            ..start();

    state "main"
    {
    }

    // we'll catch this signal every second
    catch "timer fired"
    {
        shoot();
    }

    fun shoot()
    {
        Console.print("Spaceship: SHOOT!");
    }
}
```

*Available since:* SurgeScript 7.0

Factory
-------

#### Timer

`Timer()`

Spawns a new Timer. Prefer using [cascades](../tutorials/advanced_features.md#cascades) (`..`) to configure it.

*Returns*

A new Timer.

Properties
----------

#### waitTime

`waitTime`: number.

The time, in seconds, required for the timer to fire. Defaults to one second.

#### repeat

`repeat`: boolean.

If `true`, the timer will restart automatically after it fires. Otherwise, it will [stop](#stop). Defaults to `true`.

#### frozen

`frozen`: boolean.

If `true`, the timer is frozen. A frozen timer does not fire until it's unfrozen. Defaults to `false`.

#### stopped

`stopped`: boolean, read-only.

This property is `true` if the timer is stopped (or has not started). A stopped timer does not fire.

See also: [start](#start), [stop](#stop).

#### timeLeft

`timeLeft`: boolean, read-only.

The remaining time, in seconds, for the timer to fire. If the timer is [stopped](#stopped), this is always zero.

Functions
---------

#### start

`start()`

Start the timer, or reset it if it was already started. This function does not unfreeze a [frozen](#frozen) timer.

#### stop

`stop()`

Stop the timer. A [stopped](#stopped) timer can only be restarted, unlike a [frozen](#frozen) timer.

Signals
-------

#### timer fired

The timer was just fired.

*Type:* local

*Context:* none