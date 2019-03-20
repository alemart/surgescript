Variables
=========

Variables are used to store data. SurgeScript features five primitive types (number, string, boolean, null and object handle). Variables can also point to more complex data structures (such as arrays and dictionaries), but these are always objects.

Types
-----

### Primitive types

Type|Description|Examples
----|-----------|-------
Number|A floating-point number|1, 2, 3, 3.14159
String|Some text|"Hello, world!", 'single-quoted'
Boolean|True or false|true, false
Object|Reference to an object|Application, this, [ ]
Null|An empty value|null

As an example, the script below features an object with 3 variables that may be accessed throughout the whole object:

```
object "Application"
{
    name = "Surge the Rabbit";
    age = 23;
    underage = false;

    state "main"
    {
        Console.print(name); // Surge the Rabbit
        Console.print(age); // 23
    }
}
```

### Abstract types

#### Arrays

Arrays are collections of values that you may access using a zero-based index. Example:

```
object "Application"
{
    // The 'characters' array hold 3 strings
    characters = [ "Surge", "Neon", "Charge" ];

    state "main"
    {
        // who are the characters?
        Console.print("The characters are:");
        Console.print(characters[0]); // will print Surge
        Console.print(characters[1]); // will print Neon
        Console.print(characters[2]); // will print Charge

        // how many characters?
        Console.print("Number of characters:");
        Console.print(characters.length); // will print 3
    }
}
```

Arrays have many interesting properties and operations that you can see in the [Language Reference](../reference/array.md).

#### Dictionaries

Dictionaries are collections of key-value pairs. In some programming languages, these are known as associative arrays or hash tables. Here's the syntax:

```
object "Application"
{
    // A dictionary that stores the weight (kg) of each character
    weight = {
        "Surge": 35,
        "Neon": 20,
        "Charge": 37.5,
        "Gimacian": 70
    };

    // print the weights
    state "main"
    {
        Console.print("Surge weighs " + weight["Surge"] + " kg.");
        Console.print("Neon weighs " + weight["Neon"] + " kg.");
        Console.print("Charge weighs " + weight["Charge"] + " kg.");
        Console.print("Gimacian weighs " + weight["Gimacian"] + " kg.");
    }
}
```

Dictionaries have many interesting properties and operations that you can see in the [Language Reference](../reference/dictionary.md).

> **Cautionary note**
> 
> Whenever you define an array or a dictionary, you spawn a new object. One is advised to **NOT** define arrays or dictionaries within states, because the code within states run continuously. Therefore, new objects will be created at every frame, not just once.

```
object "Application"
{
    // arr will be instantiated once the object is created
    arr = [ 1, 2, 3 ]; // RIGHT!

    state "main"
    {
        // err is instantiated at every frame of the application,
        // thus memory is wasted with duplicate objects
        err = [ "don't", "do", "this" ]; // WRONG!
    }
}
```

Casting
-------

You can use type-casting to change the type of values from number to string, from string to number, from boolean to string and so on. Casting values is as simple as calling `Number(value)`, `String(value)` or `Boolean(value)`. Example:

```
// Convert string to number
str = "2";
val = Number(str); // val is 2

// Convert number to string
val = 123;
str = String(val); // str is "123"
```

When converting an object to a string, SurgeScript will implicitly call `object.toString()`.

Scoping
-------
Variables that are defined before any state or function are *object-level* variables. They can be accessed throughout the whole object. On the other hand, variables used exclusively inside states or functions are *local variables*. This means that their lifespan is restricted to their own locus of code.

```
object "ScopeTest"
{
    name = "Surge the Rabbit"; // object-level variable

    state "main"
    {
        weight = 35; // local variable
        state = "print-data"; // change the state
    }

    state "print-data"
    {
        Console.print(name); // Surge the Rabbit
        Console.print(weight); // ERROR: weight is not defined
    }
}
```

Furthermore, *object-level* variables may only be accessed within the object. Other objects may not read or write the variables directly (in other words, variables are *private* to the objects). You need to define [functions](functions) or [getters and setters](properties) to accomplish that.
