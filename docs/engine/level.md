Level
=====

Level routines. A level is a scene in the game, stored as a .lev file in the *levels/* folder.

Whenever you spawn an object in SurgeScript, you must keep a reference to it, otherwise it will be automatically deleted by the [Garbage Collector](/reference/gc). Sometimes you want to spawn [entities](/engine/entity) in your level, but keeping references to all of them is inconvenient. In this case, you can [spawn them as children of the Level object](#spawnentity). It will keep the references for you, and thus they won't be garbage collected.

*Example*
```cs
using SurgeEngine.Actor;
using SurgeEngine.Level;
using SurgeEngine.Vector2;

object "Application"
{
    state "main"
    {
        createExplosionAt(100, 200);
        state = "wait";
    }

    state "wait"
    {
        if(timeout(1.0))
            state = "main";
    }

    fun createExplosionAt(x, y)
    {
        position = Vector2(x, y);
        return Level.spawnEntity("MyExplosion", position); // no need to keep references
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
}
```

Properties
----------

#### name

`name`: string, read-only.

The name of the level.

*Example*
```cs
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

`act`: number.

The act number (1, 2, 3...) of the current level.

*Note:* this property is writable since Open Surge 0.6.1.

#### cleared

`cleared`: boolean, read-only.

Checks if the current level has been cleared by the player. If this is `true`, a level cleared animation should be played. Although the engine provides a default animation, you may use this property to design your own.

See also: [clear](#clear).

#### file

`file`: string, read-only.

The relative path of the .lev file of the current level.

#### version

`version`: string, read-only.

The version of the level, defined in the .lev file.

#### author

`author`: string, read-only.

The author of the level, defined in the .lev file.

#### license

`license`: string, read-only.

The license of the level, defined in the .lev file.

#### music

`music`: [Music](/engine/music) object, read-only.

The music of the level.

#### bgtheme

`bgtheme`: string, read-only.

The path to the original background file (.bg), as declared in the .lev file.

#### background

`background`: string.

The path to the background file (.bg) currently in use. Use this property to change the background of the level.

#### waterlevel

`waterlevel`: number.

The y-coordinate, in world space, of the level water. Pixels not above this value are underwater.

#### spawnpoint

`spawnpoint`: [Vector2](/engine/vector2) object.

The position where the player is placed when the level starts.

#### gravity

`gravity`: number, read-only.

The acceleration of gravity, measured in pixels per second squared.

#### time

`time`: number.

Elapsed time in the level, given in seconds. The level time will be frozen if the [cleared](#cleared) flag is set or if the player is dying.

*Note:* this property is writable since Open Surge 0.6.0.

#### next

`next`: number.

The number of the next level in the current quest: 1 means the first level of the quest, 2 means the second, and so on.

#### onUnload

`onUnload`: object | `null`.

Used to specify a [function object](/tutorials/advanced_features/#function-objects) to be called when the level is unloaded.

#### debugMode

`debugMode`: boolean.

Whether or not the Debug Mode is activated.

*Available since:* Open Surge 0.6.1

Functions
---------

#### spawn

`spawn(objectName)`

Spawns an object as a child of Level. Such objects won't be garbage collected.

Use [spawnEntity](#spawnentity) instead if you intend to spawn an [entity](/engine/entity).

*Arguments*

* `objectName`: string. The name of the object to be spawned.

*Returns*

The spawned object.

#### spawnEntity

`spawnEntity(objectName, position)`

Spawns an [entity](/engine/entity) named `objectName` at a certain `position`. The entity won't be garbage collected, as in [spawn](#spawn).

*Arguments*

* `objectName`: string. The name of the entity to be spawned.
* `position`: [Vector2](/engine/vector2) object. The position of the entity.

*Returns*

The spawned entity object.

#### entity

`entity(entityId)`

[Entities](/engine/entity) spawned with [spawnEntity](#spawnentity) are automatically assigned an ID, a hexadecimal identification string. The purpose of the ID is to uniquely identify the entity in the level. This function returns the entity whose ID is `entityId`.

*Arguments*

* `entityId`: string. The ID of the entity to be retrieved.

*Returns*

The desired entity object, or `null` if there is no entity associated with the given ID.

*Example*

```cs
using SurgeEngine.Level;

//
// Level.entity() is very useful when creating setup objects, i.e.,
// objects spawned when the level is initialized. You can use it to
// tune the parameters of specific entities, call their functions, etc.
//
// The example below can be added to the startup list in the .lev file.
//
object "My Setup Object"
{
    state "main"
    {
        // setup the entity
        item = Level.entity("ab65d8fe1ebd68de"); // first, we get the entity
        if(item != null) {
            Console.print(item.__name); // then we print the object name
            //item.score = 100; // we can also change a property
        }
        else
            Console.print("Entity not found");

        // change the state
        state = "done";
    }

    state "done"
    {
    }
}
```

#### entityId

`entityId(entityObject)`

Returns the [ID](#entity) of the provided [entity](/engine/entity), if it is defined. If no such ID is defined, an empty string is returned.

*Available since:* Open Surge 0.6.1

*Arguments*

* `entityObject`: object. An entity.

*Returns*

A string with a hexadecimal identification code, or an empty string.

#### findEntity

`findEntity(entityName)`

Finds an entity named `entityName` spawned with [spawnEntity](#spawnentity).

*Available since:* Open Surge 0.6.1

*Arguments*

* `entityName`: string. The name of an entity object.

*Returns*

An entity named `entityName`, or `null` if there is none.

#### findEntities

`findEntities(entityName)`

Finds all entities named `entityName` spawned with [spawnEntity](#spawnentity).

*Available since:* Open Surge 0.6.1

*Arguments*

* `entityName`: string. The name of the entities to be searched.

*Returns*

A new array containing all entities named `entityName`. If none are found, an empty array is returned.

#### activeEntities

`activeEntities()`

Finds all active entities, i.e.: those that are near the camera, as well as all awake and detached ones.

*Available since:* Open Surge 0.6.1

*Returns*

A new array containing active entities. If none are found, an empty array is returned.

#### setup

`setup(config)`

Sets up the properties of a collection of level objects. This is a handy function typically used in a startup object of your level. Its purpose is to configure the properties of level objects. You may set the properties of groups of objects (given their names) and of individual entities alone (given their IDs).

The target properties and their respective values should be specified in the `config` [Dictionary](/reference/dictionary), passed as a parameter. Each key of `config` should be either an object name or an entity ID. Each value of the dictionary should be another dictionary specifying the properties to be set and their respective values. If an object name is specified in a key of `config`, all level objects with that name will be affected. If an entity ID is provided, only that specific entity will be affected, if it exists.

This function will search for all the specified objects in the level, so it's not supposed to be used in a loop (or in a repeating state). Furthermore, if one or more objects or properties do not exist, this function will fail silently. Therefore, make sure you type everything correctly.

*Arguments*

* `config`: [Dictionary](/reference/dictionary) object. The configuration as described.

*Example*

```cs
using SurgeEngine.Level;

//
// This is supposed to be a startup object,
// listed in the startup list of a .lev file
//
object "My Level Setup"
{
    // setup properties
    fun constructor()
    {
        Level.setup({
            "Elevator": {
                "anim": 2
            },
            "Background Exchanger": {
                "background": "themes/template.bg"
            },
            "5640353a6efd2901": {
                "someProperty": 123,
                "someOtherProperty": "hello"
            },
            "770ae26584229af2": {
                "title": "Super!!",
                "message": "Hey there! Feeling good today?",
                "buttons": [ "Yes", "No" ]
            }
        });
    }
}
```

#### restart

`restart()`

Restarts the current level.

#### quit

`quit()`

Prompts the user to see if he/she wants to quit the current level.

*Deprecated since:* Open Surge 0.6.1

#### abort

`abort()`

Quits the current level/quest without prompting the user.

#### pause

`pause()`

Pauses the game.

#### load

`load(filepath)`

Loads the specified level/quest.

* If you pass the path to a level (a .lev file in the *levels/* folder), that level will be loaded. The state of the current level (position of the [entities](/engine/entity) and so on) will be lost.

* If you pass the path to a quest (a .qst file in the *quests/* folder), that quest will be loaded and, when it's completed or aborted, the engine will redirect the user back to the level he/she was before.

*Arguments*

* `filepath`: string. Relative path of the level or quest to be loaded.

*Example*

```cs
using SurgeEngine.Level;

object "My Level Loader"
{
    fun load(id)
    {
        if(id == "beach") {
            Level.load("levels/beach_level.lev");
        }
        else if(id == "forest") {
            Level.load("levels/forest_level.lev");
        }
        else if(id == "bonus") {
            Level.next -= 1; // return to the current level after completing the bonus
            Level.load("levels/bonus_stage.lev");
        }
        else
            Console.print("Unrecognized level: " + id);
    }
}
```

#### loadAndReturn

`loadAndReturn(filepath)`

Loads the specified level or quest and, after completing or exiting the loaded scene, returns to the level the player was before. If you pass the path to a quest, this function behaves exactly like [load()](#load).

*Available since:* Open Surge 0.6.1

*Arguments*

* `filepath`: string. Relative path of the level or quest to be loaded.

*Example*

```cs
// Load a bonus stage and return to the current level if the player completes it or exits from it
Level.loadAndReturn("levels/bonus_stage.lev");
```

#### loadNext

`loadNext()`

Loads the next level in the current quest. This is the usual procedure after [clearing the level](#clear). See also: [next](#next).

#### clear

`clear()`

Sets the [cleared](#cleared) flag and disables player input.

See also: [cleared](#cleared), [undoClear](#undoclear).

#### undoClear

`undoClear()`

Reverses a previous call to [clear](#clear), disabling the [cleared](#cleared) flag and enabling player input again. This is typically used to play a cutscene after a level cleared animation or to have multiple [acts](#act) in the same level file.

See also: [cleared](#cleared), [clear](#clear), [act](#act).

*Available since:* Open Surge 0.6.1
