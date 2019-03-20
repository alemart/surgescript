Dictionary
==========

A Dictionary is a collection of key-value pairs. Dictionary keys are strings. Their corresponding values can be of any type. To create a Dictionary, use the `{ key_1: value_1, key_2: value_2, ..., key_n: value_n }` syntax rather than the `spawn()` function.

Example:

```
object "Application"
{
    dictionary = {
        "Surge": 35,
        "Neon": 20,
        "Charge": 37.5,
        "Gimacian": 70
    };

    state "main"
    {
        // usage example
        Console.print(dictionary["Surge"]); // will print 35
        Console.print(dictionary["Neon"]); // will print 20

        // will print all entries
        foreach(entry in dictionary)
            Console.print(entry.key + ": " + entry.value);

        // done!
        Application.exit();
    }
}
```

Output:

```
35
20
Surge: 35
Neon: 20
Charge: 37.5
Gimacian: 70
```

> **Note:**
> 
> Whenever you define a dictionary, you spawn a new object. One is advised to **NOT** define dictionaries within states, because the code within states run continuously. Therefore, new objects will be created at every frame of the application, not just once.

Properties
----------

#### count

`count`: number, read-only.

The number of elements in the Dictionary.

Functions
---------

#### get

`get(key)`

Gets the value of the specified key in the Dictionary. Instead of calling `get()` directly, you may equivalently use the `[ ]` operator.

*Arguments*

* `key`: string.

*Returns*

The value corresponding to the specified key, or `null` if there is no such an entry in the Dictionary.

*Example*

```
dict = { "Surge": 10 };
ten = dict["Surge"];
```

#### set

`set(key, value)`

Sets the value of the specified key in the Dictionary. Instead of calling `set()` directly, you may equivalently use the `[ ]` operator.

*Arguments*

* `key`: string.
* `value`: any type.

*Example*

```
dict = { };
dict["Surge"] = 10;
```

#### clear

`clear()`

Removes all entries from the Dictionary.

#### delete

`delete(key)`

Deletes the entry having the specified key.

*Arguments*

* `key`: string. The key of the entry to be removed.

#### has

`has(key)`

Checks if an entry having the specified key belongs to the Dictionary.

*Arguments*

* `key`: string. The key of the entry.

*Returns*

Returns `true` if the Dictionary has such an entry.

#### keys

`keys()`

Gets a collection containing the keys of the Dictionary.

*Returns*

Returns a new [Array](array) containing the keys of the Dictionary.

#### iterator

`iterator()`

Spawns an iterator.

*Returns*

An iterator to loop through the elements of the Dictionary.

#### toString

`toString()`

Converts the Dictionary to a string.

*Returns*

A string.