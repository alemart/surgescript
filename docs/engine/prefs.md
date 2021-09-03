Prefs
=====

Prefs is a SurgeEngine component used to save/load data to/from the disk (permanent storage). Prefs is a (key, value) storage that works like a [Dictionary](/reference/dictionary), but its data can be persisted between game sessions.

*Example*
```
using SurgeEngine.Prefs;

// Run the Application multiple times and see
// how the counter increments
object "Application"
{
    state "main"
    {
        Prefs["counter"] += 1;
        Console.print("counter: " + Prefs["counter"]);

        state = "idle";
    }

    state "idle"
    {
    }

    fun constructor()
    {
        if(!Prefs.has("counter"))
            Prefs["counter"] = 0;
    }
}
```

Functions
---------

#### get

`get(key)`

Gets the value of the specified key. Instead of calling `get()` directly, you may equivalently use the `[ ]` operator.

*Arguments*

* `key`: string. The key must always be a string.

*Returns*

Returns the value associated with the specified key, or `null` if there is no such value.

#### set

`set(key, value)`

Sets the value of the specified key. Instead of calling `set()` directly, you may equivalently use the `[ ]` operator.

*Arguments*

* `key`: string. The key must always be a string.
* `value`: any type. The value you want to store.

#### clear

`clear()`

Removes all entries from Prefs. Use this very carefully, because data will be lost.

#### delete

`delete(key)`

Deletes the entry having the specified key.

*Arguments*

* `key`: string. The key of the entry to be removed.

#### has

`has(key)`

Checks if a specific entry exists.

*Arguments*

* `key`: string. The key of the entry.

*Returns*

Returns `true` if the specified entry exists, `false` otherwise.

#### save

`save()`

Writes the data to secondary storage. The data isn't saved immediately after you set an entry, but it is saved on key moments. Since the data is persisted automatically, normally you don't need to call this.
