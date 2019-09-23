Plugin
======

SurgeScript features a plugin system that allows you to extend the functionalities of the language. Known as plugins or [packages](/tutorials/packages), these objects can be imported and accessed anywhere in the code. They are imported using a `using` statement. The plugin system can be accessed simply by typing `Plugin`.

Example:

```
// File: app.ss
// An application that uses StringUtils, defined in another file
using StringUtils;

object "Application"
{
    str = "alucard";

    state "main"
    {
        Console.print("Reverse a string:");
        Console.print(str);
        Console.print(StringUtils.reverse(str));
        Application.exit();
    }
}
```

```
// File: string_utils.ss
// String utilities

@Package
object "StringUtils"
{
    fun reverse(str)
    {
        buf = "";
        for(i = str.length - 1; i >= 0; i--)
            buf += str[i];
        return buf;
    }
}
```

*Available since:* SurgeScript 0.5.2

Properties
----------

#### count

`count`: number, read-only.

The number of plugins.

Functions
---------

#### spawn

`spawn(objectName)`

Spawns an object called `objectName` as a plugin. This function is not to be used directly, as SurgeScript spawns the plugins automatically for you.

*Arguments*

* `objectName`: string. The name of the object to be spawned as a plugin. It's not recommended to include special characters in this name.

#### get

`get(objectName)`

Gets a reference to the plugin named `objectName`. The `[ ]` operator can be used instead of the `get()` function, as in `Plugin[objectName]`.

Rather than using this function, it's recommended to import the desired object using the `using MyObject;` statement at the beginning of your code. Your object will be available as `MyObject`.

*Arguments*

* `objectName`: string. The name of the plugin.

*Example*

```
using StringUtils;

// ...

assert(Plugin["StringUtils"] === StringUtils);
assert(Plugin.StringUtils === StringUtils);
```