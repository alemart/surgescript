Level
=====

Level routines. A level is a scene in the game, represented by a .lev file in the *levels/* folder.

Whenever you spawn an object in SurgeScript, you should keep a reference to it, otherwise it will be deleted by the garbage collector. Sometimes, you may want to spawn [entities](entity) in your level, but keeping references to all of them may be inconvenient. If this is your case, you can [spawn them as children of the Level object](#spawn). It will keep references of the entities for you; therefore, they won't be garbage collected.

*Example*
```
using SurgeEngine.Level;
using SurgeEngine.Player;

object "Application"
{
    state "main"
    {
        if(timeout(1))
            state = "explode";
    }

    state "explode"
    {
        Level.spawn("MyExplosion"); // no need to keep references
        state = "main";
    }
}

object "MyExplosion" is "entity", "disposable", "private"
{
    actor = Actor("MyExplosion");

    state "main"
    {
        if(actor.animation.finished)
            destroy()
    }

    fun constructor()
    {
        // set initial position
        player = Player.active;
        actor.transform.worldX = player.transform.worldX;
        actor.transform.worldY = player.transform.worldY;
    }
}
```

Properties
----------

#### name

`name`: string, read-only.

The name of the level.

*Example*
```
using SurgeEngine.Level;

// Will display the name of the level
object "Application"
{
    state "main"
    {
        Console.print(Level.name);
        state = "done";
    }

    state "done"
    {
    }
}
```

#### act

`act`: number, read-only.

The act number (1, 2, 3...) of the current level.

#### version

`version`: string, read-only.

The version of the level, defined in the .lev file.

#### author

`author`: string, read-only.

The author of the level, defined in the .lev file.

#### license

`license`: string, read-only.

The license of the level, defined in the .lev file.

#### file

`file`: string, read-only.

The relative path of the .lev file of the current level.

#### music

`music`: [Music](music) object, read-only.

The music of the level.

#### waterlevel

`waterlevel`: number.

The y-coordinate, in world space, of the level water. Pixels not above this value are underwater.

#### cleared

`cleared`: boolean, read-only.

Checks if the current level has been cleared by the player. If this is `true`, a *level cleared* animation should be played. Although the engine provides a default animation, you may use this property to design your own. See also: [clear()](#clear).

Functions
---------

#### spawn

`spawn(objectName)`

Spawns an object as a child of Level. Such objects won't be garbage collected. Please note that the spawned object is required to be an [entity](entity).

*Arguments*

* `objectName`: string. The name of the object to be spawned.

*Returns*

The spawned object.

#### restart

`restart()`

Restarts the current level.

#### quit

`quit()`

Prompts the user to see if he/she wants to quit the current level.

#### abort

`abort()`

Quits the current level/quest without prompting the user.

#### pause

`pause()`

Pauses the game.

#### load

`load(filepath)`

Loads the specified level/quest.

* If you pass the path to a level (a .lev file in the *levels/* folder), the specified level will be loaded. The state of the current level (position of the [entities](entity) and so on) will be lost.

* If you pass the path to a quest (a .qst file in the *quests/* folder), the specified quest will be loaded and, when it's completed, the engine will redirect the user back to the level he/she was before. This might be useful for creating bonuses, configuration screens, and so on.

*Arguments*

* `filepath`: string. Relative path of the level or quest to be loaded.

*Example*
```
using SurgeEngine.Level;

object "LevelLoader"
{
    fun load(id)
    {
        if(id == "beach")
            Level.load("levels/my_beach_level.lev");
        else if(id == "forest")
            Level.load("levels/my_forest_level.lev");
        else
            Console.print("Unrecognized level: " + id);
    }
}
```

#### clear

`clear()`

Clears the level without actually changing it. Once the level is cleared, a *level cleared* animation is played. See also: [cleared](#cleared).

#### finish

`finish()`

Loads the next level in the current quest. This is the usual procedure after [clearing the level](#clear).
