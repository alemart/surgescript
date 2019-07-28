EventList
=========

An EventList is an [event](/engine/event) that triggers multiple events when triggered.

Factory
-------

#### Events.EventList

`Events.EventList(list)`

Spawns an EventList.

*Arguments*

* `list`: [Array](/reference/array) object. A list containing zero or more events.

*Returns*

A new EventList that triggers the specified events at once, in the order they have been listed.

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
            "Event Trigger 1": {
                "onTrigger": EventList([
                    FunctionEvent("Print").withArgument("We're triggering"),
                    FunctionEvent("Print").withArgument("multiple events"),
                    FunctionEvent("Print").withArgument("at the same time!")
                ])
            }
        });
    }
}
```

Functions
---------

#### call

`call()`

Triggers the event.