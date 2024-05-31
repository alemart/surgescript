Event
=====

An event is a [function object](/tutorials/advanced_features/#function-objects) with zero parameters and no return value that is tagged *"event"*. Events are used to run code when triggered.

Events let you seamlessly connect [entities](/engine/entity) and write cause and effect relationships. Suppose you have an on/off switch in a level. Whenever that switch is pressed by the player, you want something to happen: raise the water level, open a door, make it rain, teleport the player, and so on. Events are important because they let you decouple the source of the happening (the pressing of the switch) from the results that are experienced (e.g., the raising of the water).

Events let you reuse your entities in multiple ways and for multiple purposes. You may design an entity and make it trigger an event, but you do not need to specify in advance what the event actually is. Rather, the concrete event (what must happen) will be configured in your [level setup](/engine/level/#setup).

There are multiple types of events. In practice, you'll use specialized events such as [EntityEvent](/engine/entityevent) and [FunctionEvent](/engine/functionevent) when configuring your entities. *Event* is an empty event that does nothing when triggered, but it should be used when designing your entities, as in the example below.

*Example*

```cs
// -----------------------------------------------------------------------------
// Using Events
//
// Note that cause and effect are split into different scripts!
// -----------------------------------------------------------------------------

// This script contains an object that will trigger an event whenever a certain
// button is pressed (e.g., the jump button).
using SurgeEngine.Player;
using SurgeEngine.Events.Event;

object "Button Detector" is "awake", "entity"
{
    public onButtonPress = Event();
    public button = "fire1";

    state "main"
    {
        input = Player.active.input;
        if(input.buttonPressed(button))
            onButtonPress();
    }
}

// -----------------------------------------------------------------------------

// The code below is a different script containing a level setup object.
// Make sure to link it in your .lev file!
using SurgeEngine.Level;
using SurgeEngine.Events.FunctionEvent;

object "My Level Setup"
{
    fun constructor()
    {
        Level.setup({
            "Button Detector": {
                "button": "fire1", // "fire1" is the jump button
                "onButtonPress": FunctionEvent("Print").withArgument("You pressed the button!")
            }
        });
    }
}
```

Factory
-------

#### Events.Event

`Events.Event()`

Spawns an empty Event.

*Returns*

A new Event.

Functions
---------

#### call

`call()`

Triggers the event. All event types implement this function. It takes no arguments and it returns no value other than `null`.
