Loops
=====

While loops
-----------

In a while loop, you execute a certain block of code while a condition is true. The syntax is as follows:

```
while(condition) {
    // block of code
    // to be executed
}
```

The example below counts from 1 to 10:

```
i = 1;
while(i <= 10) {
    Console.print(i);
    i = i + 1;
}
```

For loops
---------

A for loop is a more controlled repetition structure when compared to the while loop. For loops support an initialization command, a condition and an increment command. The syntax is as follows:

```
for(initialization; condition; increment) {
    // block of code
    // to be executed
}
```

The code just displayed is equivalent to:

```
initialization;
while(condition) {
    // block of code
    // to be executed

    increment;
}
```

The example below counts from 1 to 10:

```
for(i = 1; i <= 10; i++) {
    Console.print(i);
}
```

Foreach
-------

Foreach loops are used to iterate throughout collections (such as Arrays). Basically: for each element *x* in the collection, do something with *x*. The syntax is as follows:

```
foreach(element in collection) {
    // block of code
    // to be executed
}
```

The example below counts from 1 to 10:

```
collection = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
foreach(x in collection) {
    Console.print(x);
}
```