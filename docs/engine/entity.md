Entity
======

Entity is not an object per-se. It's a class of objects. An entity is an object that is present in the game world (examples: an item, a NPC, a baddie, and so on). Abstract objects or components are **not** entities. **In Open Surge, all objects that are tagged *"entity"* are considered to be entities.**

Entites have special treatment in Open Surge. They are deactivated automatically if they get too far off camera. Behavior can be changed by adding different tags to your entities. Additionally, entities can have components associated to it, so they can be fully customized.

> **Tip**
> 
> During level design, entities may be placed in world space using the editor palette. The icon of the entity will be the first animation (animation 0) of the sprite that has the name of the entity.

Tags
----

#### entity

Tells Open Surge that the object is an entity and should be treated as such.

```cs
// In the example below, MyExplosion (entity) has an Actor component
// that gives it a visible form (a sprite) in world space.
using SurgeEngine.Actor;

object "MyExplosion" is "entity", "private", "disposable"
{
    actor = Actor("MyExplosion");

    state "main"
    {
        if(actor.animation.finished)
            destroy();
    }
}
```

#### awake

Tells Open Surge to **not** disable the object if it gets too far off camera.

#### detached

The object will **not** follow the camera - it will be rendered in screen space.

#### disposable

Will destroy the object automatically if it gets too far off camera.

#### private

Private entities cannot be spawned via the level editor (they will be hidden).


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

If an entity\* that has been placed in the level via the editor gets too far off camera, it will be deactivated and repositioned back to its spawn point (i.e., the place where it was originally). Whenever that happens, the engine will call this function if it's available in your entity. You may want to use this to reset the entity back to its initial state.

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

If declared, function `onLeaveEditor()` will be called whenever the player leaves the level editor and returns to the game. This might be useful to reconfigure your objects.
