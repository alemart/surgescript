EntityEvent
===========

An EntityEvent is an [event](/engine/event) that calls a function of an [entity](/engine/entity) when triggered.

Factory
-------

#### Events.EntityEvent

`Events.EntityEvent`

Spawns an EntityEvent.

*Arguments*

* `entityID`: string. The ID of an entity.

*Returns*

A new EntityEvent linked to the specified entity.

*Example*

```cs
// This is a level setup object. Make sure to link it in your .lev file!
using SurgeEngine.Level;
using SurgeEngine.Events.EntityEvent;

object "My Level Setup"
{
    fun constructor()
    {
        Level.setup({
            "Event Trigger 1": {
                "onTrigger": EntityEvent("aeb587eed1057a5e").willCall("open")
            }
        });
    }
}
```

Functions
---------

#### willCall

`willCall(functionName)`

Specifies the name of the function that will be called.

*Arguments*

`functionName`: string. The name of the function that will be called when the event is triggered.

*Returns*

The EntityEvent object.

#### withArgument

`withArgument(data)`

Adds an argument to the EntityEvent. Arguments added to the EntityEvent will be passed to the entity when the event is triggered - in the order they have been added. To add multiple arguments, call this function multiple times.

*Arguments*

`data`: any. The argument to be added.

*Returns*

The EntityEvent object.

#### call

`call()`

Triggers the event.