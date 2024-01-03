Player
======

The Player object is used to control a specific player/character. Player objects are tagged *player*.

**Note:** in Open Surge, companion objects can be used to give new abilities to players. Companion objects must be indicated in the character definition files (.chr) and are implemented as regular scripts. The engine will automatically spawn the companions as children of the appropriate Player objects.

*Example*

```cs
object "Display player name" is "companion"
{
    player = parent; // since this object is configured as a
                     // companion, parent is the reference
                     // to the appropriate Player object

    state "main"
    {
        Console.print("I am a companion of " + player.name);
    }
}
```

Global
------

#### Player

`Player(playerName)`

Gets the Player object associated with a character in the level. If a player with the specified name doesn't exist in the level, there will be an error.

See also: [exists](#playerexists).

*Arguments*

* `playerName`: string. The name of a character, as defined in a *.chr* file stored in the *characters/* folder.

*Returns*

A Player object.

*Example*
```cs
using SurgeEngine.Player;

// Gives Surge 1 collectible each second
// Just place it in your level
object "Collectible Giver" is "entity", "awake"
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

#### Player.get

`Player[i]`

Gets a Player object by its sequence number, as defined in the *players* entry of the level file.

See also: [count](#playercount).

*Arguments*

* `i`: number. An integer between `0` and `Player.count - 1`, inclusive.

*Returns*

A Player object.

#### Player.exists

`Player.exists(playerName)`

Checks if a player named `playerName` exists in the level.

*Arguments*

* `playerName`: string. The name of a character.

*Returns*

Returns `true` if such a player exists in the level.

#### Player.active

`Player.active`: [Player](#) object.

The player currently in focus.

See also: [focus](#focus), [hasFocus](#hasfocus), [focusable](#focusable).

*Example*
```cs
using SurgeEngine.Player;

// Tells the name of the active player
// Just place it in the level
object "Who am I" is "entity", "awake"
{
    state "main"
    {
        player = Player.active;
        Console.print("I am " + player.name);
        destroy();
    }
}
```

#### Player.count

`Player.count`: number, read-only.

The number of players in the level.

#### Player.initialLives

`Player.initialLives`: number, read-only.

The initial number of lives set by the engine.

#### Player.mode

`Player.mode`: string.

Player modes change gameplay mechanics. They affect how players interact in a level. There may be multiple players in a level, but [only one has focus](#playeractive) at any given time. The table below summarizes the available modes:

| Mode | Use case | Damage | Input | Items |
| ---- | -------- | ------ | ----- | ----- |
| <nobr>`"cooperative"`</nobr> <nobr>*since v0.6.1*</nobr> <nobr>*default mode*</nobr> | Best suited when all players are human-controlled. | A [life](#lives) is lost if any player is [killed](#kill). If a player is [hit](#hit), it receives [focus](#focus). | Only the [focused player](#playeractive) receives direct user [input](#input). All respond to [simulated input](/engine/input#simulatebutton). | No special treatment. |
| <nobr>`"classic"`</nobr> <nobr>*since v0.6.1*</nobr> | Best suited when non-focused players are AI-controlled. | Non-focused players are [invulnerable](#invulnerable) and [immortal](#immortal). A life is lost only if the focused player is killed. | Only the focused player receives direct user input. All respond to simulated input. | Non-focused players are made [secondary](#secondary). |

*Available since:* Open Surge 0.6.1

Properties
----------

#### id

`id`: number, read-only.

A number that uniquely identifies the player in the Level.

*Available since:* Open Surge 0.6.1

#### name

`name`: string, read-only.

The name of the character.

#### direction

`direction`: number, read-only.

This number will be +1 if the player is facing right, or -1 if facing left.

#### input

`input`: [Input](/engine/input) object, read-only.

Reference to the Input object of the Player.

*Example*
```cs
using SurgeEngine.Player;

object "Application"
{
    state "main"
    {
        player = Player.active;

        if(player.input.buttonDown("right"))
            Console.print("Player is holding right");
    }
}
```

#### animation

`animation`: [Animation](/engine/animation) object, read-only.

Reference to the Animation object of the Player.

#### anim

`anim`: number.

A shortcut to `animation.id`: an integer corresponding to the animation number.

See also: [Animation](/engine/animation), [id](/engine/animation#id).

#### anchor

`anchor`: [Vector2](/engine/vector2) object.

A shortcut to `animation.anchor`.

See also: [anchor](/engine/animation#anchor).

*Available since:* Open Surge 0.6.0

#### hotSpot

`hotSpot`: [Vector2](/engine/vector2) object.

A shortcut to `animation.hotSpot`.

See also: [hot spot](/engine/animation#hotspot).

*Available since:* Open Surge 0.6.0. In versions prior to 0.6.0, you may get the hot spot using the [Animation](/engine/animation#hotspot) object.

#### actionSpot

`actionSpot`: [Vector2](/engine/vector2) object.

A shortcut to `animation.actionSpot`.

See also: [action spot](/engine/animation#actionspot).

*Available since:* Open Surge 0.6.0

#### actionOffset

`actionOffset`: [Vector2](/engine/vector2) object, read-only.

A shortcut to `animation.actionOffset`.

See also: [action offset](/engine/animation#actionoffset).

*Available since:* Open Surge 0.6.0

#### attacking

`attacking`: boolean, read-only.

Is the player attacking? (jumping, rolling, and so on)

#### midair

`midair`: boolean, read-only.

Is the player midair?

#### stopped

`stopped`: boolean, read-only.

Is the player stopped?

#### walking

`walking`: boolean, read-only.

Is the player walking?

#### running

`running`: boolean, read-only.

Is the player running?

#### jumping

`jumping`: boolean, read-only.

Is the player jumping?

#### springing

`springing`: boolean, read-only.

Is the player playing the "springing" animation, displayed just after hitting a spring?

#### braking

`braking`: boolean, read-only.

Is the player braking?

#### balancing

`balancing`: boolean, read-only.

Is the player balancing on a ledge?

#### waiting

`waiting`: boolean, read-only.

Is the player waiting? (happens after remaining stopped for a few seconds)

#### rolling

`rolling`: boolean, read-only.

Is the player rolling?

#### charging

`charging`: boolean, read-only.

Is the player charging a rolling movement?

#### pushing

`pushing`: boolean, read-only.

Is the player pushing a wall?

#### hit

`hit`: boolean, read-only.

Is the player getting hit (i.e., losing collectibles or a shield)?

#### blinking

`blinking`: boolean, read-only.

Is the player blinking? (happens just after getting hit)

#### dying

`dying`: boolean, read-only.

Is the player dying or drowning?

#### drowning

`drowning`: boolean, read-only.

Is the player drowning (i.e., dying underwater)?

#### breathing

`breathing`: boolean, read-only.

Is the player breathing an air bubble underwater?

#### crouchingDown

`crouchingDown`: boolean, read-only.

Is the player crouching down?

#### lookingUp

`lookingUp`: boolean, read-only.

Is the player looking up?

#### winning

`winning`: boolean, read-only.

Is the player displaying the "winning" animation (after clearing a level)?

#### secondsToDrown

`secondsToDrown`: number, read-only.

The number of seconds to drown, if underwater.

#### topspeed

`topspeed`: number, read-only.

The maximum speed that the player can reach when running naturally, without rolling and with no external forces applied. Given in pixels per second.

#### capspeed

`capspeed`: number, read-only.

The maximum speed that the player can reach by normal means (e.g., rolling downhill), not considering external forces. Given in pixels per second.

*Available since:* Open Surge 0.6.1

#### hlockTime

`hlockTime`: number, read-only.

The time, in seconds, in which the player will not respond to horizontal input when on the ground. Defaults to zero.

See also: [hlock](#hlock).

*Available since:* Open Surge 0.6.1

#### transform

`transform`: [Transform](/engine/transform) object, read-only.

The transform of the Player.

#### collider

`collider`: [Collider](/engine/collider) object, read-only.

A collider associated with this Player.

#### shield

`shield`: string | `null`.

One of the following: *"shield"*, *"fire"*, *"thunder"*, *"water"*, *"acid"*, *"wind"* or `null` (if there is no shield).

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

#### aggressive

`aggressive`: boolean.

If set to `true`, the [attacking](#attacking) flag will be `true` and the player will be able to hit the baddies just by touching them, regardless if he/she is jumping or not. Defaults to `false`.

*Available since:* Open Surge 0.5.1

#### invulnerable

`invulnerable`: boolean.

If set to `true`, the player will be invulnerable to damage. It's like having an invisible shield that is permanent. The player may still be killed if crushed, by falling into a pit, by drowning, or by calling [kill](#kill).

*Available since:* Open Surge 0.6.1

#### immortal

`immortal`: boolean.

If an immortal player appears to be killed, it will appear to be ressurrected on its spawn point without losing a [life](#lives).

*Available since:* Open Surge 0.6.1

#### secondary

`secondary`: boolean.

A secondary player plays a secondary role and interacts with items in different ways. It cannot smash item boxes, activate goal signs, etc. This flag is best used with AI-controlled players. Defaults to `false`.

*Available since:* Open Surge 0.6.1

#### focusable

`focusable`: boolean.

A focusable player can receive focus. Defaults to `true`.

See also: [focus](#focus), [hasFocus](#hasfocus), [Player.active](#playeractive).

*Available since:* Open Surge 0.6.1

*Note:* if only a single player exists in the level, then that player will have focus regardless of the value of this flag.

#### breathTime

`breathTime`: number.

The maximum number of seconds the player can remain underwater without breathing.

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

#### speed

`speed`: number.

The speed of the player, in pixels per second. This will be mapped automatically to [gsp](#gsp) or [xsp](#xsp), depending on whether the player is touching the ground or not.

#### gsp

`gsp`: number.

Ground speed, in pixels per second. Prefer using [speed](#speed) instead, since `gsp` is only meaningful if the player is touching the ground.

#### xsp

`xsp`: number.

Horizontal speed, in pixels per second (useful while midair). Prefer using [speed](#speed) instead.

#### ysp

`ysp`: number.

Vertical speed, in pixels per second.

#### angle

`angle`: number.

The angle of the player, in degrees. The same as `transform.angle`.

#### slope

`slope`: number, read-only.

The angle detected by the physics system, in degrees. Unlike [angle](#angle), slope is read-only and does not feature any smoothing for visual aesthetics. Its purpose is to be a helper for physics calculations.

*Available since:* Open Surge 0.5.2

#### width

`width`: number, read-only.

The width of the player sprite, in pixels.

#### height

`height`: number, read-only.

The height of the player sprite, in pixels.

Functions
---------

#### bounce

`bounce(hazard | null)`

Makes the player bounce after smashing a hazard. The trajectory of the movement will be computed according to the position of the hazard. If no hazard is present, you may pass `null` as the argument.

*Arguments*

* `hazard`: [Actor](/engine/actor) object. The hazard.

*Example*
```cs
using SurgeEngine.Player;
using SurgeEngine.Actor;
using SurgeEngine.Collisions.CollisionBox;

object "Shield Box" is "entity"
{
    actor = Actor("Shield Box");
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

* `hazard`: [Actor](/engine/actor) object. The hazard.

#### getHit

`getHit(hazard | null)`

Makes the player get hit. Call it whenever the player gets hit by a hazard. The hit movement will be calculated according to the position of the hazard. If no hazard is present, you may pass `null` as the argument.

*Arguments*

* `hazard`: [Actor](/engine/actor) object. The hazard.

#### kill

`kill()`

Kills the player.

#### breathe

`breathe()`

Makes the player breathe (underwater only).

#### restore

`restore()`

Restores the player to a vulnerable state.

*Available since:* Open Surge 0.6.1

#### springify

`springify()`

Activates the "springing" activity.

#### roll

`roll()`

Makes the player roll.

#### hlock

`hlock(seconds)`

Locks the horizontal controls of the player for the specified time. Left and right input will be ignored during that time.

See also: [hlockTime](#hlocktime).

*Arguments*

* `seconds`: number. How long the horizontal controls of the player will stay locked? This usually should be set to a value no higher than `0.5`.

*Note:* since Open Surge 0.6.1, the locking will not take place if the player is midair.

#### focus

`focus()`

Focuses on the player. The focused player is controlled by the user. Only one player has focus at any given time.

See also: [hasFocus](#hasfocus), [focusable](#focusable), [Player.active](#playeractive).

#### hasFocus

`hasFocus()`

Does the player have focus?

See also: [focus](#focus), [focusable](#focusable), [Player.active](#playeractive).

*Returns*

Returns `true` if the player has focus, `false` otherwise.

#### moveBy

`moveBy(dx, dy)`

Moves the player by a (`dx`,`dy`) offset after the physics update of the current framestep. This is similar to calling `player.transform.translateBy(dx, dy)` in `lateUpdate()`.

See also: [lateUpdate](/engine/entity#lateupdate), [translateBy](/engine/transform#translateby).

*Available since:* Open Surge 0.6.1

*Arguments*

* `dx`: number. Horizontal offset in pixels.
* `dy`: number. Vertical offset in pixels.

*Example*

```cs
// Let's compare player.moveBy() to
// player.transform.translateBy()
dx = 5;
dy = 0;

// this moves the player AFTER the physics update
player.moveBy(dx, dy);

// this moves the player BEFORE the physics update
// (immediately), unless you call it in lateUpdate()
player.transform.translateBy(dx, dy);
```

#### move

`move(offset)`

Moves the player by the given `offset` after the physics update of the current framestep. Calling `player.move(offset)` is equivalent to calling `player.moveBy(offset.x, offset.y)`.

See also: [moveBy](#moveby).

*Available since:* Open Surge 0.6.1

*Arguments*

* `offset`: [Vector2](/engine/vector2) object. Offset vector.

#### transformInto

`player.transformInto(characterName)`

Transforms the player into the character named `characterName`. The transformation succeeds only if there is such a character.

*Available since:* Open Surge 0.6.1

*Arguments*

* `characterName`: string. The name of a character, as defined in a .chr file.

*Returns*

Returns `true` if the transformation is successful. Transforming a player into itself is considered successful, but nothing happens in this case.

```cs
// this player will become Surge
player.transformInto("Surge");

// this player will become Tux
player.transformInto("Tux");

// the following transformation will fail:
if(!player.transformInto("non-existent-character"))
    Console.print("No transformation took place!");

// After a successful transformation:
// - player.name will change;
// - player.id will not change!
```
