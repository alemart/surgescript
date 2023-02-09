Player
======

The Player object is used to control a specific player/character. Player objects are tagged *player*. That might be useful when dealing with collisions.

**Note:** in Open Surge, companion objects can be used to give new abilities to players. Companion objects must be indicated in the character definition files (.chr) and are implemented as regular scripts. The engine will automatically spawn the user-defined companion objects as children of the correct Player objects.

*Example*

```cs
using SurgeEngine.Audio.Sound;

//
// This is a dash move that should be configured as a
// companion object in a character definition file (.chr)
//
// When you are stopped, hold up and press jump to charge.
// Release up after 0.5 second and you'll gain a nice boost!
//
object "My Peel Out" is "companion"
{
    charge = Sound("samples/charge.wav");
    release = Sound("samples/release.wav");
    player = parent; // since this object is configured as a
                     // companion, parent is the reference
                     // to the correct Player object

    speed = 720;     // dash speed, in pixels/second

    // capture the event
    state "main"
    {
        if(player.lookingUp) {
            if(player.input.buttonPressed("fire1")) {
                charge.play();
                state = "charging";
            }
        }
    }

    // charging the dash
    state "charging"
    {
        player.anim = 2; // running animation
        player.animation.speedFactor = 1.85;
        player.frozen = true; // disable physics (temporarily)

        // ready to go?
        if(player.input.buttonReleased("up")) {
            if(timeout(0.5)) {
                player.gsp = speed * player.direction; // dash!!!
                release.play();
            }
            player.frozen = false; // enable physics
            state = "main";
        }
        else if(player.input.buttonPressed("fire1"))
            charge.play();
    }
}
```

Factory
-------

#### Player

`Player(playerName)`

Gets the Player object associated with a character in the level.

*Arguments*

* `playerName`: string. The name of the character, as defined in a *.chr* file stored in the *characters/* folder.

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

#### Player.active

`Player.active`

The active player, i.e., the one currently in focus.

*Returns*

A Player object.

*Example*
```cs
using SurgeEngine.Player;

// Tells the name of the active player
// Just place it in the level
object "Who am I" is "entity", "awake"
{
    state "main"
    {
        Console.print("I am " + Player.active.name);
        destroy();
    }
}
```

#### Player.get

`Player[i]`

Gets a Player object by its sequence number, as defined in the *players* entry of the level file.

*Arguments*

* `i`: number. An integer between `0` and `Player.count - 1`, inclusive.

*Returns*

A Player object.

#### Player.count

`Player.count`

Player count.

*Returns*

The number of players in the level.

#### Player.initialLives

`Player.initialLives`

The initial number of lives set by the engine.

*Returns*

The initial number of lives.

Properties
----------

#### name

`name`: string, read-only.

The name of the character.

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
        if(Player.active.input.buttonDown("right"))
            Console.print("Player is holding right");
    }
}
```

#### animation

`animation`: [Animation](/engine/animation) object, read-only.

Reference to the Animation object of the Player.

#### anim

`anim`: number.

A shortcut to `animation.id`: an integer corresponding to the animation number. See also: [Animation](/engine/animation), [id](/engine/animation#id).

#### anchor

`anchor`: [Vector2](/engine/vector2) object.

A shortcut to `animation.anchor`. See also: [anchor](/engine/animation#anchor).

*Available since:* Open Surge 0.6.0

#### hotSpot

`hotSpot`: [Vector2](/engine/vector2) object.

A shortcut to `animation.hotSpot`. See also: [hot spot](/engine/animation#hotspot).

*Available since:* Open Surge 0.6.0. In versions prior to 0.6.0, you may get the hot spot using the [Animation](/engine/animation#hotspot) object.

#### actionSpot

`actionSpot`: [Vector2](/engine/vector2) object.

A shortcut to `animation.actionSpot`. See also: [action spot](/engine/animation#actionspot).

*Available since:* Open Surge 0.6.0

#### actionOffset

`actionOffset`: [Vector2](/engine/vector2) object, read-only.

A shortcut to `animation.actionOffset`. See also: [action offset](/engine/animation#actionoffset).

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

The maximum speed the player can reach without items (i.e., running naturally), in pixels per second.

#### direction

`direction`: number, read-only.

Direction will be +1 if the player is facing right, -1 if facing left.

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

#### springify

`springify()`

Activates the "springing" activity.

#### roll

`roll()`

Makes the player roll.

#### hlock

`hlock(seconds)`

Locks the horizontal controls of the player for a few `seconds` (left and right input will be ignored during that time).

*Arguments*

* `seconds`: number. Specify how long the horizontal controls of the player will stay locked.

#### focus

`focus()`

Focuses on the player. The focused player is controlled by the user. Only one player can have focus at any given time.

#### hasFocus

`hasFocus()`

Does the player have focus?

*Returns*

Returns `true` if the player has focus, `false` otherwise.
