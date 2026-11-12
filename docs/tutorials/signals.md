Signals
=======

Signals provide an easy way for objects to communicate with each other without tight coupling. Objects can *emit* signals, which are then *caught* by others. With this feature, you can easily handle events of interest in your game.

*Available since:* SurgeScript 7

Basics
------

Consider this example: when a button is pressed, you want to open a door. In this scenario, a button object emits a signal communicating that it has been pressed. A door object then catches that signal and opens itself in response to that occurrence. Button and door need not be tightly coupled in the code: they communicate with each other by means of a signal.

In code, we proceed as follows:

1. We declare the signal
2. The emitter object emits the signal
3. Other objects catch the signal

```cs
// this is the declaration of the signal named "button pressed"
signal "button pressed" { }

// we declare that object "My Button" can emit signals named "button pressed";
// hence, "My Button" is the emitter in this example
object "My Button" emits "button pressed"
{
    state "main"
    {
    }

    fun press()
    {
        Console.print("The button was pressed");

        // we emit the signal when appropriate
        emit("button pressed");
    }
}

// object "My Door" will catch the signal
object "My Door"
{
    state "main"
    {
    }

    // this is a signal handler. All existing objects with a matching signal
    // handler will be notified whenever the specified signal is emitted
    catch "button pressed"
    {
        open();
    }

    fun open()
    {
        Console.print("A door was opened");
    }
}

object "Application"
{
    button = spawn("My Button");
    door = spawn("My Door");

    state "main"
    {
        if(timeout(2)) {
            button.press();
            state = "done";
        }
    }

    state "done"
    {
    }
}
```

The output of this program is:

```
The button was pressed
A door was opened
```

As soon as `button.press()` is called by the application, the `"button pressed"` signal is emitted by the button. That signal is immediately caught by the door, which opens itself.

Note that `button` and `door` do not hold references to each other. Moreover, if we extend the above example to include multiple doors, then **all doors** will catch the signal:

```cs
...

object "Application"
{
    button = spawn("My Button");
    doors = [ spawn("My Door"), spawn("My Door"), spawn("My Door") ];

    ...
}
```

This extended example spawns 3 doors. Since each door prints a message, the output of the program is:

```
The button was pressed
A door was opened
A door was opened
A door was opened
```

Note what just happened: with the same structure of the previous example, a single emitter (button) notified multiple objects (doors) of an event of interest (the button was pressed).

!!! note "Convention for signal handlers"

    The `catch` blocks must be written after the states and before the functions of an object.

Naming convention
-----------------

All signals have a name. These names should briefly describe an occurrence that has just happened. When naming your signals, abide by the following convention as much as possible:

_name = subject + verb in the past tense_

The *subject* refers to the subject of the occurrence, whereas the *verb* refers to the occurrence itself. Use lowercase letters for both. Take a look at some examples of names that follow and that do not follow the convention:

| ✅ Use | ❌ Avoid |
|--------|----------|
| `"button pressed"` | `"press button"` |
| `"powerup collected"` | `"collect powerup"` |
| `"level cleared"` | `"clear"` |
| `"player killed"` | `"player death"` |
| `"explosion finished"` | `"EXPLOSION FINISHED"` |
| `"life gained"` | `"Extra Life"` |
| `"points gained"` | `"add to score"` |
| `"energy recharged"` | `"Update UI"` |

Context
-------

Radio stations example

Anonymous objects

Local signals
-------------

Timer example