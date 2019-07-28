EventChain
==========

An EventChain is an [event](/engine/event) that triggers other events, sequentially, as in a chain of events.

Factory
-------

#### Events.EventChain

`Events.EventChain(chain)`

Spawns an EventChain.

*Arguments*

* `chain`: [Array](/reference/array) object. A list containing zero or more events.

*Returns*

A new EventChain that triggers the specified events, one at a time. The first time the EventChain is triggered, the first event of the `chain` will be triggered. The second time the EventChain is triggered, the second event of the `chain` will be triggered, and so on.

**Note:** if the EventChain ever triggers its last event, from that moment onwards it will always trigger its last event (unless you make it loop).

*Example*

```
// This is a level setup object. Make sure to link it in your .lev file!
using SurgeEngine.Level;
using SurgeEngine.Events.EventList;
using SurgeEngine.Events.FunctionEvent;

object "My Level Setup"
{
    fun constructor()
    {
        Level.setup({
            "Switch": {
                "sticky": false,
                "onActivate": EventChain([
                    FunctionEvent("Print").withArgument("First time"),
                    FunctionEvent("Print").withArgument("Second time"),
                    FunctionEvent("Print").withArgument("Third time"),
                    FunctionEvent("Print").withArgument("Enough!")
                ])
            }
        });
    }
}
```

Functions
---------

#### willLoop

`willLoop()`

Make the EventChain loop. When the last event of the chain is triggered, the next event to be triggered will be the first one.

*Returns*

The EventChain object.

#### call

`call()`

Triggers the event.