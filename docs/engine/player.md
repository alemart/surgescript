Player
======

The Player object is used to control a specific player/character. Player objects are tagged *player*. That might be useful when dealing with collisions.

Factory
-------

#### Player

`Player(playerName | playerId)`

Gets the Player object associated with a certain character.

*Arguments*

* `playerName`: string. The name of the character (defined in the *characters/* folder).
* `playerId`: number. The ID of the character (as defined in the *.lev* file), an integer between `0` and `Player.count - 1`, inclusive.

*Returns*

A Player object.

*Example*
```
using SurgeEngine.Player;

// Gives Surge 1 collectible each second
// Just place it in your level
object "CollectibleGiver" is "entity", "awake"
{
    player = Player("Surge");

    state "main"
    {
        if(timeout(1))
            state = "give collectible";
    }

    state "give collectible"
    {
        player.collectibles = player.collectibles + 1;
        state = "main";
    }
}
```

#### Player.active

`Player.active`

The active player, i.e., the one currently in focus.

*Returns*

A Player object.

*Example*
```
using SurgeEngine.Player;

// Tells the name of the active player
object "Who am I"
{
    state "main"
    {
        Console.print("I am " + Player.active.name);
        destroy();
    }
}
```

#### Player.count

`Player.count`

Player count.

*Returns*

The number of players in the level.

Properties
----------

#### name

`name`: string, read-only.

The name of the character.

#### activity

`activity`: string, read-only.

The current state / activity of the player. One of the following: *"stopped"*, *"walking"*, *"running"*, *"jumping"*, *"springing"*, *"rolling"*, *"charging"*, *"pushing"*, *"gettinghit"*, *"dying"*, *"braking"*, *"balancing"*, *"drowning"*, *"breathing"*, *"ducking"*, *"lookingup"*, *"waiting"*, *"winning"*.

```
using SurgeEngine.Player;

// Place this inside your level to see the
// current activity of the active player
object "ActivityDebugger" is "awake", "entity"
{
    state "main"
    {
        player = Player.active;
        Console.print(player.activity);
    }
}
```

#### anim

`anim`: number.

A shortcut to `animation.id`: an integer corresponding to the animation number.

#### animation

`animation`: [Animation](animation) object, read-only.

Reference to the Animation object of the Player.

#### attacking

`attacking`: boolean, read-only.

Is the player attacking? (jumping, rolling, and so on)

#### midair

`midair`: boolean, read-only.

Is the player midair?

#### secondsToDrown

`secondsToDrown`: number, read-only.

The number of seconds to drown, if underwater.

#### direction

`direction`: number, read-only.

Direction will be +1 if the player is facing right, -1 if facing left.

#### width

`width`: number, read-only.

The width of the player sprite, in pixels.

#### height

`height`: number, read-only.

The height of the player sprite, in pixels.

#### transform

`transform`: [Transform](transform) object, read-only.

The transform of the Player.

#### collider

`collider`: [Collider](collider) object, read-only.

A collider associated with this Player.

#### shield

`shield`: string.

One of the following: *"none"*, *"shield"*, *"fire"*, *"thunder"*, *"water"*, *"acid"*, *"wind"*.

#### invincible

`invincible`: boolean.

Used to make the player invincible. Defaults to `false`.

#### turbo

`turbo`: boolean.

Turbo mode (increases player speed). Defaults to `false`.

#### frozen

`frozen`: boolean.

Disable/enable player movement. Defaults to `false`.

#### underwater

`underwater`: boolean.

Get underwater / out of water. Defaults to `false`.

#### layer

`layer`: string.

The current layer of the player. One of the following: *"green"*, *"yellow"*, *"default"*. This property tells you which bricks will be sensed, depending on their layer.

#### visible

`visible`: boolean.

Should the player sprite be rendered? Defaults to `true`.

#### collectibles

`collectibles`: number.

The number of collectibles, an integer shared between all player objects.

#### lives

`lives`: number.

The number of lives, an integer shared between all player objects.

#### score

`score`: number.

The score of the player, an integer value shared between all player objects.

#### gsp

`gsp`: number.

Ground speed, in pixels per second.

#### xsp

`xsp`: number.

Horizontal speed, in pixels per second (useful while midair).

#### ysp

`ysp`: number.

Vertical speed, in pixels per second.


Functions
---------

#### bounce

`bounce(hazard | null)`

Makes the player bounce after smashing a hazard. The trajectory of the movement will be computed according to the position of the hazard. If no hazard is present, you may pass `null` as the argument.

*Arguments*

* `hazard`: [Actor](actor) object. The hazard.

*Example*
```
using SurgeEngine.Player;
using SurgeEngine.Actor;
using SurgeEngine.Collisions.CollisionBox;

object "ShieldBox" is "entity"
{
    actor = Actor("ShieldBox");
    shieldCollider = CollisionBox(32,32);

    state "main"
    {
        player = Player.active;
        if(player.attacking) {
            if(player.collider.collidesWith(shieldCollider)) {
                if(player.midair)
                    player.bounce(actor);
                player.shield = "fire";
                destroy();
            }
        }
    }
}
```

#### bounceBack

`bounceBack(hazard)`

Makes the player bounce after smashing a hazard. If the player comes at the hazard from below, the player is thrown downwards. If not, the player is thrown upwards. The `hazard` parameter is mandatory.

*Arguments*

* `hazard`: [Actor](actor) object. The hazard.

#### hit

`hit(hazard | null)`

Makes the player get hit. Call it whenever the player gets hit by a hazard. The hit movement will be calculated according to the position of the hazard. If no hazard is present, you may pass `null` as the argument.

*Arguments*

* `hazard`: [Actor](actor) object. The hazard.

#### kill

`kill()`

Kills the player.

#### breathe

`breathe()`

Makes the player breathe (underwater only).

#### springify

`springify()`

Activates the "springing" activity.

#### roll

`roll()`

Makes the player roll.

#### focus

`focus()`

Focuses on the player. The focused player is controlled by the user. Only one player can have focus at any given time.

#### hasFocus

`hasFocus()`

Does the player have focus?

*Returns*

Returns `true` if the player has focus, `false` otherwise.
