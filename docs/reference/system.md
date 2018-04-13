System
======

The root object. Used to gather data from the language and to access special objects. It can be accessed by typing `System`.

Properties
----------

* `version` is the version of the language runtime (string).
* `info` tells you information about the SurgeScript runtime (string).
* `tags` is a reference to the [Tag System](tags.md).
* `gc` is a reference to the [Garbage Collector](gc.md).
* `objectCount` is the total number of objects at this moment.

Functions
---------

#### exit

`exit()`

Exits the Application.

*Example*

```
// This utility displays System.info
object "Application"
{
    state "main"
    {
        Console.print(System.info);
        System.exit();
    }
}
```

#### destroy

`destroy()`

The same as `exit()`.