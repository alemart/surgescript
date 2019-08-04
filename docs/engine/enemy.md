Enemy
=====

The Enemy [behavior](/engine/behavior) makes the associated [entity](/engine/entity) behave like an enemy. It will hit the player if touched, unless the player is attacking (jumping, rolling, etc.) In this case the enemy will be destroyed with an explosion, giving the player a certain score.

Although the Enemy object generates the described behavior, it is not a concrete enemy itself. You may use it to script your own baddies.

*Example*

```
//
// HOW TO SCRIPT A BADDIE:
//
// 0. Make sure you have the graphics and the sprite file (.spr) ready,
//    before you begin with SurgeScript
// 1. Your object should be tagged (at least): "entity", "enemy"
// 2. Spawn an Actor for the graphics and an Enemy object for the behavior.
//
using SurgeEngine.Actor;
using SurgeEngine.Behaviors.Enemy;
using SurgeEngine.Behaviors.Platformer;

object "My Baddie" is "entity", "enemy"
{
    actor = Actor("My Baddie"); // handles the graphics
    enemy = Enemy(); // handles the behavior
    platformer = Platformer().walk(); // make it walk

    state "main"
    {
        enemy.score = 100;
    }
}
```

Optionally, you may define functions `onEnemyAttack(player)` and `onEnemyDestroy(player)` on your entity if you want to catch the events: the enemy has attacked a player and the enemy has been destroyed by the player, respectively.

Factory
-------

#### Behaviors.Enemy

`Enemy()`

Spawns an Enemy behavior.

*Returns*

An Enemy behavior object.

Properties
----------

#### score

`score`: number.

The score given to the player when the enemy is defeated.

#### invincible

`invincible`: boolean.

Is the enemy invincible? An invincible enemy hits the player even when jumping, rolling, etc. - except if the player is also invincible. Defaults to `false`.

#### collider

`collider`: [Collider](/engine/collider) object, read-only.

A collider associated with the enemy.

Functions
---------

#### setBounds

`setBounds(left, top, right, bottom)`

Set the boundaries of the collider. All coordinates, given in pixels, are relative to the hot spot of the entity. These boundaries are computed automatically, but you may use this function if you need to adjust them.

*Arguments*

* `left`: number. Given in pixels.
* `top`: number. Given in pixels.
* `right`: number. Given in pixels.
* `down`: number. Given in pixels.

*Returns*

Returns the Enemy behavior itself.
