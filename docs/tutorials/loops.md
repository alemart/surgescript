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

The following example prints nothing. The while loop is skipped because the condition is initially false:

```cs
i = 20;
while(i <= 10) { // this condition is false
    Console.print(i);
    i = i + 1;
}
```

Do-while loops
--------------

Do-while loops are similar to while loops, except that the block of code is executed at least once, unconditionally. It is repeated while a condition is true. Syntax:

```cs
do {
    // block of code
    // to be executed
} while(condition);
```

The example below counts from 1 to 10:

```cs
i = 1;
do {
    Console.print(i);
    i = i + 1;
} while(i <= 10);
```

The following example prints the number 20. Since the condition is false, the block of code is executed only once:

```cs
i = 20;
do {
    Console.print(i);
    i = i + 1;
} while(i <= 10); // this condition is false
```

Do-while loops are available since SurgeScript 0.6.0.

For loops
---------

A for loop is a more controlled repetition structure compared to a while loop. For loops support an initialization command, a condition and an increment command. The syntax is as follows:

```cs
for(initialization; condition; increment) {
    // block of code
    // to be executed
}
```

The for loop above is equivalent to:

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

Since SurgeScript 0.6.1, the initialization, condition and increment expressions may be empty. Empty initialization and increment expressions do nothing. An empty condition is interpreted as `true`, meaning that the loop will keep repeating until you [break out of it](#break) with a `break` or with a `return` statement.

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

!!! tip "Implementing your own iterable collections"

    The foreach loop can be used with any iterable collections, not only [Arrays](/reference/array) and [Dictionaries](/reference/dictionary). You may even [implement your own!](/tutorials/advanced_features#iterators)

Break
-----

Use the `break` statement to terminate the innermost enclosing loop. The statement following the loop will be executed immediately afterwards. Example:

```cs
i = 0;

while(i < 10) {
    if(i == 5)
        break; // break out of the loop when i is equal to 5

    i = i + 1;
}

// This line will be executed immediately after breaking out of the loop
Console.print(i); // Output: 5
```

Continue
--------

Use the `continue` statement to terminate the current iteration of the innermost enclosing loop and to start the next iteration. The exact behavior of `continue` depends on the type of the loop:

* In `while` and `do-while` loops, `continue` jumps to the loop condition. The loop condition is reevaluated.
* In a `for` loop, `continue` jumps to the increment expression. Next, the loop condition is reevaluated.
* In a `foreach` loop, `continue` jumps to the start of the next iteration.

Example:

```cs
i = 0;

while(i < 5) {
    i = i + 1;

    if(i == 3)
        continue; // jump back to the condition i < 5 when i is equal to 3

    Console.print(i);
}

// Expected output: 1 2 4 5
// Number 3 was skipped
```

A similar example with a `for` loop:

```cs
for(i = 1; i <= 5; i++) {
    if(i == 3)
        continue; // jump to i++ when i is equal to 3

    Console.print(i);
}
```

An analogous example with a `foreach` loop:

```cs
foreach(i in [1,2,3,4,5]) {
    if(i == 3)
        continue; // jump to the next iteration when i is equal to 3

    Console.print(i);
}
```