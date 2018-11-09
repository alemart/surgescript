Player
======

The Player object is used to control a specific player/character. Player objects are tagged *player*. That might be useful when dealing with collisions.

Factory
-------

#### Player

`SurgeEngine.Player(playerName | playerId)`

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
object "Collectible Bonus"
{
    player = Player("Surge");

    state "main"
    {
        if(timeout(1))
            state = "+1 collectible";
    }

    state "+1 collectible"
    {
        player.collectibles = player.collectibles + 1;
        state = "main";
    }
}
```

#### Player.active

`SurgeEngine.Player.active`

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

`SurgeEngine.Player.count`

Player count.

*Returns*

The number of players in the level.

Read-Only Properties
--------------------

* `name`: string. The name of the character.
* `activity`: string. The current state / activity of the player. One of the following: *"stopped"*, *"walking"*, *"running"*, *"jumping"*, *"springing"*, *"rolling"*, *"charging"*, *"pushing"*, *"gettinghit"*, *"dying"*, *"braking"*, *"balancing"*, *"drowning"*, *"breathing"*, *"ducking"*, *"lookingup"*, *"waiting"*, *"winning"*.
* `attacking`: boolean. Is the player attacking? (jumping, rolling, and so on)
* `midair`: boolean. Is the player midair?
* `secondsToDrown`: number. The number of seconds to drown, if underwater.
* `direction`: number. It will be +1 if the player is facing right, or -1 if facing left.
* `width`: number. The width of the player sprite, in pixels.
* `height`: number. The height of the player sprite, in pixels.
* `transform`: [Transform2D](../reference/transform2d) object. The transform of the Player.
* `collider`: [Collider](collider) object. A collider associated with this Player.

Read-Write Properties
---------------------

* `anim`: number. An integer value corresponding to the animation number.
* `shield`: string. One of the following: *"none"*, *"shield"*, *"fire"*, *"thunder"*, *"water"*, *"acid"*, *"wind"*.
* `invincible`: boolean. Used to make the player invincible. Defaults to `false`.
* `turbo`: boolean. Turbo mode (increases player speed). Defaults to `false`.
* `frozen`: boolean. Disable/enable player movement. Defaults to `false`.
* `underwater`: boolean. Get underwater / out of water. Defaults to `false`.
* `visible`: boolean. Should the player sprite be rendered? Defaults to `true`.
* `collectibles`: number. The number of collectibles, an integer shared between all player objects.
* `lives`: number. The number of lives, an integer shared between all player objects.
* `score`: number. The score of the player, an integer value shared between all player objects.
* `gsp`: number. Ground speed, in pixels per second.
* `xsp`: number. X-speed, in pixels per second (useful in midair).
* `ysp`: number. Y-speed, in pixels per second.


Functions
---------

#### bounce

`bounce(hazard | null)`

Makes the player bounce after smashing a hazard. The trajectory of the movement will be computed according to the position of the hazard. If no hazard is present, you may pass `null` as the argument.

*Arguments*

* `hazard`: [Actor](actor) object. The hazard.

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
