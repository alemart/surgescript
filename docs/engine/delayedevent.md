DelayedEvent
============

A DelayedEvent is an [event](/engine/event) that, when triggered, triggers another event after a specified time has passed.

Factory
-------

#### Events.DelayedEvent

`Events.DelayedEvent(event)`

Spawns a DelayedEvent.

*Arguments*

* `event`: [event](/engine/event) object. The event to be delayed.

*Returns*

A new DelayedEvent that, when triggered, will trigger the specified event after a delay.

*Example*

```
// This is a level setup object. Make sure to link it in your .lev file!
using SurgeEngine.Level;
using SurgeEngine.Events.EventList;
using SurgeEngine.Events.DelayedEvent;
using SurgeEngine.Events.FunctionEvent;

object "My Level Setup"
{
    fun constructor()
    {
        Level.setup({
            "Event Trigger 1": {
                "onTrigger": EventList([
                    FunctionEvent("Print").withArgument("Hello!"),
                    DelayedEvent(
                        FunctionEvent("Print").withArgument("This is...")
                    ).willWait(2.0), // wait 2 seconds before triggering this
                    DelayedEvent(
                        FunctionEvent("Print").withArgument("SurgeScript!")
                    ).willWait(4.0) // wait 4 seconds before triggering this
                ])
            }
        });
    }
}
```

Functions
---------

#### willWait

`willWait(seconds)`

Set the delay to trigger the event.

*Arguments*

`seconds`: number. The delay, in seconds.

*Returns*

The DelayedEvent object.

#### call

`call()`

Triggers the event.