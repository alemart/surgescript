Entity
======

An entity is an object that generally is present in the virtual world. Examples include: a pickup item, a gimmick, a non-playable-character, and so on. Entites have special treatment:

* Entities can have [components](/tutorials/components) associated to it. They can be fully customized.
* By default, they are automatically disabled and moved back to their initial position in the world if they get too far off camera.
* Behavior can be changed by adding special [tags](#tags) them.
* The engine will automatically call special [functions](#functions) if you define them.

!!! abstract "Definition"
    
    A SurgeScript object is considered to be an entity if it meets all of the following conditions:
    
    1. the object is [tagged](/tutorials/tags) "entity"
    2. the object is a [direct child](/tutorials/object_tree) of [Level](/engine/level) or a direct child of another entity
    
    Objects that are direct children of entities but that are not tagged "entity" are considered to be [components](/tutorials/components). Components are meant to modify the entities in some way.
    
    Components may not have any entities as descendants. For example: a child of a component is not considered to be an entity, even if it's tagged "entity".
    
    Level setup objects and player companion objects are special cases. They are always considered to be entities, regardless if they are tagged "entity" or not, for backwards compatibility purposes.

!!! tip
    
    During level design, entities may be placed in world space using the editor palette. The icon of the entity will be the first animation (animation 0) of the sprite that has the name of the entity.

Tags
----

#### entity

Mark the object as an entity.

```cs
// In the example below, entity "My Explosion" has an Actor
// component that gives it visible form: a sprite.
using SurgeEngine.Actor;

object "My Explosion" is "entity", "private", "disposable"
{
    actor = Actor("My Explosion");

    state "main"
    {
        if(actor.animation.finished)
            destroy();
    }
}
```

#### private

Private entities cannot be spawned via the level editor - they will be hidden.

#### awake

Don't disable the object nor move it back to its initial position if it gets too far off camera.

#### disposable

The engine will automatically destroy the object if it gets too far off camera.

#### detached

The object will not follow the camera. It will be rendered in screen space.



Functions
---------

#### lateUpdate

`lateUpdate()`

If `lateUpdate()` is implemented in your entity, it will be called after all other objects have been updated. This handy function helps order script execution. For example: a routine that controls the [Camera](/engine/camera) should be implemented in `lateUpdate()`, since it's guaranteed to be executed after other entities have been updated.

*Available since:* Open Surge 0.6.1

*Example*
```cs
using SurgeEngine.Camera;
using SurgeEngine.Player;

object "My Simple Camera" is "awake", "entity"
{
    fun lateUpdate()
    {
        player = Player.active;
        Camera.position = player.transform.position;
    }
}
```

#### onReset

`onReset()`

If an entity that is not awake nor detached gets too far off camera, it will be automatically disabled and moved back to its initial position. Whenever that happens, the engine will call this function if it's available in your entity. You may use this function to reset the entity back to its initial state.

**Note\*:** entities tagged as awake or detached are not affected.

*Example*
```cs
using SurgeEngine.UI.Text;

// The object below is a simple counter that gets
// reseted whenever it gets too far off camera.
object "My Test Counter" is "entity"
{
    label = Text("default");
    counter = 0;

    state "main"
    {
        Console.print("Starting the counter...");
        state = "wait";
    }

    state "wait"
    {
        label.text = counter;
        if(timeout(1.0))
            state = "increment";
    }

    state "increment"
    {
        counter++;
        state = "wait";
    }

    // Without implementing function onReset() below,
    // the counter would retain its state.
    fun onReset()
    {
        counter = 0;
        state = "main";
    }
}
```

#### onLeaveEditor

`onLeaveEditor()`

If declared, function `onLeaveEditor()` will be called whenever the player leaves the level editor and returns to the game. This may be useful to reconfigure your objects.
