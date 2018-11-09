Entity
======

Entity is not an object per-se. Rather, it's a class of objects. An entity is an object that is present in world space (for example: an item, a NPC, a baddie, and so on). Abstract objects or components are **not** entities. In Open Surge, all objects that are tagged *"entity"* are considered to be entities.

Entites have special treatment in Open Surge. They are deactivated automatically if they get too far off camera. Behavior can be changed by adding different tags to your entities. Additionally, entities can have components associated to it, so they can be fully customized.

**Tip:** during level design, entities may be placed in world space using the editor palette. The icon of the entity will be the first animation (animation 0) of the sprite that has the name of the entity.

Tags
----

* `entity`: tells Open Surge that the object is an entity and should be treated as such.
* `awake`: tells Open Surge to **not** disable the object if it gets too far off camera.
* `detached`: the object will **not** follow the camera - it will be rendered in screen space.
* `disposable`: will destroy the object automatically if it gets too far off camera.
* `private`: private entities cannot be spawned via the level editor (they will be hidden).

Example
-------

```
// In the example below, MyExplosion (entity) has an Actor component
// that gives it a visible form (a sprite) in world space.
using SurgeEngine.Actor;

object "MyExplosion" is "entity", "disposable"
{
    actor = Actor("MyExplosion");

    state "main"
    {
        if(timeout(1))
            destroy();
    }
}
```
