FunctionEvent
=============

A FunctionEvent is an [event](/engine/event) that calls a [function object](/tutorials/advanced_features/#function-objects) when triggered.

Factory
-------

#### Events.FunctionEvent

`Events.FunctionEvent(objectName)`

Spawns a FunctionEvent.

*Arguments*

* `objectName`: string. The name of a function object.

*Returns*

A new FunctionEvent that, when triggered, calls the specified function object.

*Example*

```cs
// This is a level setup object. Make
// sure to link it in your .lev file!
using SurgeEngine.Level;
using SurgeEngine.Events.FunctionEvent;

object "My Level Setup"
{
    fun constructor()
    {
        Level.setup({
            "Event Trigger 1": {
                "onTrigger": FunctionEvent("Give 1up")
            }
        });
    }
}

// -----------------------------------------

// This is a function object that
// gives the player one extra life.
using SurgeEngine.Player;
using SurgeEngine.Audio.Music;

object "Give 1up"
{
    jingle = Music("samples/1up.ogg");

    fun call()
    {
        Player.active.lives += 1;
        jingle.play();
    }
}
```

Functions
---------

#### withArgument

`withArgument(data)`

Adds an argument to the FunctionEvent. Arguments added to the FunctionEvent will be passed to the function object when the event is triggered - in the order they have been added. To add multiple arguments, call this function multiple times.

*Arguments*

`data`: any. The argument to be added.

*Returns*

The FunctionEvent object.

#### call

`call()`

Triggers the event.