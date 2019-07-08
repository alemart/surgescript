Behavior
========

A behavior is an object that, when spawned as a child of an [entity](/engine/entity), adds a specific behavior to that entity. Example: you can add a behavior to give a [Directional Movement](/engine/directional_movement) to an entity, so you don't need to code that movement yourself. Although the use of behaviors is not mandatory, they greatly simplify the development of your entities.

An important aspect of using behaviors is that you can create your own. You can also reuse them in many of your entities and share them with others. Behavior objects should be built in a generic way, so that they can be reused throughout multiple entities.

Behavior objects must be tagged *behavior* and are supposed to be spawned as direct children of entities. They should also implement the protocol (properties, methods) described below. The engine provides a few behaviors by default, and they have been documented in this manual.

*Example*

```
//
// The example below shows an entity with 2 behaviors:
//
// - Enemy: it makes the entity behave like an enemy
//   (it can hit the player, or be defeated)
//
// - DirectionalMovement: it makes the entity move
//   in a certain direction, with a certain speed
//
using SurgeEngine.Actor;
using SurgeEngine.Vector2;
using SurgeEngine.Behavior.Enemy;
using SurgeEngine.Behavior.DirectionalMovement;

// Mosquito is a flying baddie
object "Mosquito" is "entity", "enemy"
{
    actor = Actor("Mosquito");
    enemy = Enemy();
    movement = DirectionalMovement();

    state "main"
    {
        actor.hflip = true; // flip the sprite horizontally
        movement.direction = Vector2.left;
        movement.speed = 60;
    }
}
```

Properties
----------

#### entity

`entity`: object, read-only.

The [entity](/engine/entity) associated with this behavior. This is a convenient alias to the parent object.

#### enabled

`enabled`: boolean.

Is the behavior enabled? The default value is `true`, i.e., behaviors are enabled by default.
