Loops
=====

While loops
-----------

In a while loop, you execute a certain block of code while a condition is true. The syntax is as follows:

```cs
while(condition) {
    // block of code
    // to be executed
}
```

The example below counts from 1 to 10:

```cs
i = 1;
while(i <= 10) {
    Console.print(i);
    i = i + 1;
}
```

For loops
---------

A for loop is a more controlled repetition structure when compared to the while loop. For loops support an initialization command, a condition and an increment command. The syntax is as follows:

```cs
for(initialization; condition; increment) {
    // block of code
    // to be executed
}
```

The code just displayed is equivalent to:

```cs
initialization;
while(condition) {
    // block of code
    // to be executed

    increment;
}
```

The example below counts from 1 to 10:

```cs
for(i = 1; i <= 10; i++) {
    Console.print(i);
}
```

Foreach
-------

Foreach loops are used to iterate throughout iterable collections (such as [Arrays](/reference/array) and [Dictionaries](/reference/dictionary)). Basically: for each element `x` in the iterable collection, do something with `x`. The syntax is as follows:

```cs
foreach(element in collection) {
    // block of code
    // to be executed
}
```

The example below counts from 1 to 10:

```cs
collection = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
foreach(x in collection) {
    Console.print(x);
}
```

The example below iterates over a [Dictionary](/reference/dictionary):

```cs
dictionary = { "a": 1, "b": 2, "c": 3 };
foreach(entry in dictionary) {
    Console.print(entry.key);
    Console.print(entry.value);
}
```

> **Implementing your own iterable collections**
>
> The foreach loop can be used with any iterable collections, not only [Arrays](/reference/array) and [Dictionaries](/reference/dictionary). You may even [implement your own!](/tutorials/advanced_features#iterators)
