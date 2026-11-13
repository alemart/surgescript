Signals
=======

Signals provide an easy way for objects to communicate without holding references to each other. Objects can *emit* signals, which are then *caught* by others. With this feature, you can easily respond to events of interest in your game.

*Available since:* SurgeScript 7

Basics
------

Consider this example: when a button is pressed, you want to open a door. In this scenario, a button object emits a signal communicating that it has just been pressed. A door object then catches that signal and opens itself in response to that occurrence. Button and door need not be tightly coupled in the code: they communicate with each other by means of a signal.

In code, we proceed as follows:

1. We declare the signal
2. The emitter object emits the signal
3. Other objects catch the signal

```cs
// this is the declaration of the signal named "button pressed"
signal "button pressed" { }

// "My Button" is the emitter object. We declare that it can emit
// signals named "button pressed"
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
        button.press();
        state = "done";
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

!!! info "Convention for signal handlers"

    The `catch` blocks must be written after the states and before the functions of an object.

!!! note "Signal names"

    As of SurgeScript 7, the signal name passed to `emit()` must be a string literal. This helps to determine at compile-time all signals that can be emitted by an object.

Naming convention
-----------------

All signals have a name. These names should briefly describe an action or change that has just occurred. When naming your signals, abide by the following convention as much as possible:

_signal name = subject + verb in the past tense_

The *subject* refers to the subject that has received the action or whose internal state has changed. The *verb* refers to the action or change itself. Use lowercase letters for both. Take a look at some examples of names that follow and that do not follow the convention:

| ✅ Use | ❌ Avoid |
|--------|----------|
| `"button pressed"` | `"press button"` |
| `"powerup collected"` | `"collect powerup"` |
| `"level cleared"` | `"clear"` |
| `"player killed"` | `"player death"` |
| `"boss defeated"` | `"BOSS DEFEATED"` |
| `"life gained"` | `"Extra Life"` |
| `"points gained"` | `"add to score"` |
| `"energy recharged"` | `"Update UI"` |

You may read the signal name `"boss defeated"` as _a boss was defeated just now_. You may read other signal names similarly.

Context
-------

It's often useful to attach data to a signal. For example, if a button is pressed, we may wish to know which character pressed it. If the player has scored points, we may wish to know just how many points. We can think of many other examples. In any case, the additional data, if present, depends on the nature of the signal. We call such additional data the *context* of the signal.

Suppose that radio stations of a game world emit signals as soon as starting to broadcast a program. Those signals include additional information: the name and the host of the program. Let's declare a signal with this context:

```cs
signal "broadcast started"
{
    name,
    host
}
```

This means that signals named `"broadcast started"` carry a context with two fields: `name` and `host`. Note that you can specify multiple fields by separating them with commas.

Whenever a radio station emits a `"broadcast started"` signal, it must include the appropriate context. Such context must be provided as an object featuring two [properties](./properties.md): `name` and `host`. When emitting any signal, we typically specify the context as an [anonymous object](./anonymous_objects.md), and we pass it to `emit()` immediately after the signal name. Let's see an example:

```cs
object "Gamedev Radio Station" emits "broadcast started"
{
    ONE_SECOND = 1;
    ONE_MINUTE = 60 * ONE_SECOND;
    ONE_HOUR = 60 * ONE_MINUTE; // one hour == 3600 seconds

    state "main"
    {
        state = "broadcasting stories";

        /* let's emit a signal
           "broadcast started" is the name of the signal
           { name = ... , host = ... } is the context */
        emit("broadcast started", {
            name = "Gamedev Stories",
            host = "Surge the Rabbit"
        });
    }

    state "broadcasting stories"
    {
        if(timeout(ONE_HOUR)) {
            state = "broadcasting music";
            emit("broadcast started", {
                name = "Video Game Music Show",
                host = "Neon the Squirrel"
            });
        }
    }

    state "broadcasting music"
    {
        if(timeout(ONE_HOUR)) {
            state = "broadcasting news";
            emit("broadcast started", {
                name = "Gamedev News",
                host = "Tux the Penguin"
            });
        }
    }

    state "broadcasting news"
    {
        if(timeout(ONE_HOUR)) {
            state = "broadcasting stories";
            emit("broadcast started", {
                name = "Gamedev Stories",
                host = "Surge the Rabbit"
            });
        }
    }
}
```

In this example, object `"Gamedev Radio Station"` broadcasts three programs: Gamedev Stories, Gamedev News, and Video Game Music Show, each with a different host. Individual radio receivers may pick them up by accessing the context of the emitted signals in a `catch` block (also known as a _signal handler_).

The `signal` keyword, when typed inside a `catch` block, references a special [Signal](../reference/signal.md) object containing data about the received signal. Its most important properties are:

| Property | Description |
| ---------|-------------|
| `signal.context` | The context of the signal |
| `signal.emitter` | The object that emitted the signal |

In our example, `signal.context` is an object with two fields: `name` and `host`. Moreover, `signal.emitter` is the `"Gamedev Radio Station"` object which emitted the signal. Let's see how individual radio receivers may pick up those signals:

```cs
object "My Radio Receiver"
{
    state "main"
    {
    }

    catch "broadcast started"
    {
        name = signal.context.name;
        host = signal.context.host;

        Console.print("New program: " + name);
        Console.print("Presented by: " + host);
    }
}
```

The above radio receiver works fine if there is only a single station, but what if there are more? Since it picks up signals named `"broadcast started"` emitted by **any** object, this means that it picks up the programs broadcasted by **all** radio stations. This is not what we want, for a radio receiver should be tuned only to a single station of choice. Luckily, this is easy to fix. We can simply check if `signal.emitter` is our station of choice:

```cs
object "My Radio Receiver"
{
    public station = "Gamedev Radio Station";

    state "main"
    {
    }

    catch "broadcast started"
    {
        if(signal.emitter.__name == station) {
            name = signal.context.name;
            host = signal.context.host;

            Console.print("New program: " + name);
            Console.print("Presented by: " + host);
        }
    }
}
```

Now, tuning the receiver to a different station is just a matter of changing its `station` variable.

!!! info "Required fields"

    Object `signal.context` is guaranteed to contain all fields specified in the declaration of the signal. It is an error to emit a signal without all the required fields. If no fields are declared, then there is no context and `signal.context` will be `null`.

!!! note

    The special `signal` object may not be accessed outside of a signal handler (i.e., outside of a `catch` block). Storing a reference to it is an error that leads to undefined behavior.

Local signals
-------------

Signals are _global_ by default
Timer example