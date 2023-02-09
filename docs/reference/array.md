Array
=====

An Array is a collection of values organized in sequencial fashion. To instantiate an Array, use the `[ value_1, value_2, ..., value_n ]` syntax rather than the `spawn()` function.

Example:

```cs
object "Application"
{
    // The 'characters' array hold 3 strings
    characters = [ "Surge", "Neon", "Charge" ];

    state "main"
    {
        // who are the characters?
        Console.print("The characters are:");
        Console.print(characters[0]);
        Console.print(characters[1]);
        Console.print(characters[2]);

        // how many characters?
        Console.print("Number of characters: " + characters.length);
    }
}
```

Output:

```
The characters are:
Surge
Neon
Charge
Number of characters: 3
```

> **Note:**
> 
> Whenever you define a array, you spawn a new object. One is advised to **NOT** define arrays within states, because the code within states run continuously. Therefore, new objects will be created at every frame, not just once.

Properties
----------

#### length

`length`: number, read-only.

The number of elements in the Array.

Functions
---------

#### get

`get(index)`

Gets the specified element of the Array. Instead of calling `get()` directly, you may equivalently use the `[ ]` operator.

*Arguments*

* `index`: integer number between `0` and `this.length - 1`, inclusive.

*Returns*

The Array element at position `index` (0-based).

*Example*

```cs
characters = [ "Surge", "Neon", "Charge" ];

...

surge = characters[0];  // first element ("Surge")
neon = characters[1];   // second element ("Neon")
charge = characters[2]; // third element ("Charge")
```

#### set

`set(index, value)`

Sets to `value` the element of the Array at position `index`. Instead of calling `set()` directly, you may equivalently use the `[ ]` operator.

*Arguments*

* `index`: integer number between `0` and `this.length - 1`, inclusive.
* `value`: any type. The new value to be placed on the Array.

*Example*

```cs
characters = [ "Surge", "Neon", "Charge" ];
characters[0] = "Gimacian";
Console.print(characters[0]); // Gimacian
```


#### push

`push(value)`

Adds a new element to the end of the Array.

*Arguments*

* `value`: any type. The element to be added to the Array.

*Example*

```cs
characters = [ "Surge", "Neon", "Charge" ];
characters.push("Gimacian");
Console.print(characters); // [ "Surge", "Neon", "Charge", "Gimacian" ]
```

#### pop

`pop()`

Removes the last element of the Array and returns it.

*Returns*

The removed element.

#### shift

`shift()`

Removes the first element of the Array and returns it.

*Returns*

The removed element.

#### unshift

`unshift(value)`

Adds a new element to the beginning of the Array.

*Arguments*

* `value`: any type. The element to be added to the Array.

#### clear

`clear()`

Clears the array. It will no longer hold any elements and its length will be set to zero.

*Available since:* SurgeScript 0.5.3

#### indexOf

`indexOf(value)`

Search the Array for element `value` and return its position.

*Arguments*

* `value`: any type. The element to be searched for.

*Returns*

The position of the searched element - a number between `0` and `this.length - 1`, inclusive. If the desired element is not found, this function returns `-1`.

*Example*

```cs
characters = [ "Surge", "Neon", "Charge" ];
...
two = characters.indexOf("Neon"); // 2
gimacian = characters.indexOf("Gimacian"); // -1
if(gimacian < 0)
    Console.print("Not found");
```

#### sort

`sort(cmpFun)`

Sorts the Array. If no comparison [functor](/tutorials/advanced_features#functors) is provided (`cmpFun` is `null`), the Array will be sorted in ascending order.

*Arguments*

* `cmpFun`: object | null. This [functor](/tutorials/advanced_features#functors) implements function `call(a, b)` that compares two array elements as described in the example below.

*Returns*

The sorted array. The returned array is the same array as you called `sort()` on; it's not a copy.

*Example*

```cs
// this example will print the elements
// of the Array in ascending order
object "Application"
{
    arr = [ 3, 9, 4, 8, 5, 6, 7, 1, 2, 0 ];

    state "main"
    {
        // sort and print the Array
        arr.sort(null);
        Console.print(arr);
        Application.exit();
    }
}
```

Output: `[ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ]`

```cs
// this example will print the elements
// of the Array in descending order
object "Application"
{
    arr = [ 3, 9, 4, 8, 5, 6, 7, 1, 2, 0 ];
    cmp = spawn("Sort.OrderByDesc");

    state "main"
    {
        // sort and print the Array
        arr.sort(cmp);
        Console.print(arr);
        Application.exit();
    }
}

object "Sort.OrderByDesc"
{
    // This function compares two
    // elements of the Array: a and b.
    //
    // It should return a number:
    //   < 0 if a must come BEFORE b
    //   = 0 if a and b are equivalent
    //   > 0 if a must come AFTER b
    fun call(a, b)
    {
        // sort in descending order
        return b - a;
    }
}
```

Output: `[ 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 ]`

#### reverse

`reverse()`

Reverses the order of the elements in the Array.

#### shuffle

`shuffle()`

Shuffles the elements of the Array, placing its elements at random spots.

#### iterator

`iterator()`

Spawns an iterator.

*Returns*

An iterator to loop through the elements of the Array.

#### toString

`toString()`

Converts the Array to a string.

*Returns*

A string.
