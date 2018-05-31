Plugin
======

SurgeScript features a plugin system that allows you to extend the functionalities of the language. Plugins are objects that can be accessed anywhere in the code. They can be imported using a `using` statement. Also, the plugin system can be accessed simply by typing `Plugin`.

Example:

```
// File: app.ss

// Plugins are used to extend the language. You can
// import them into your source file using the
// "using" statement:
using StringUtils;

// An application that uses the imported plugin
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

// Plugins are objects annotated with "@Plugin".
// They are spawned automatically and can be
// accessed anywhere in the code.
@Plugin
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

* `count`: number. The number of spawned plugins.

Functions
---------

#### spawn

`spawn(pluginName)`

Spawns object `pluginName` as a plugin. This function is not to be used directly, as SurgeScript spawns the plugins automatically for you. SurgeScript recognizes an object as a plugin when you annotate it with `@Plugin`.

*Available since:* SurgeScript 0.5.2

*Arguments*

* `pluginName`: string. The name of the object to be spawned as a plugin. It's not recommended to include special characters in the plugin name.

#### get

`get(pluginName)`

Gets a reference to the plugin named `pluginName`. The `[ ]` operator can be used instead of the `get()` function, as in `Plugin[pluginName]`.

Rather than using this function, it's recommended to import the desired plugin using the `using myPlugin;` statement at the beginning of your code. Your plugin will be available as `myPlugin`.

*Available since:* SurgeScript 0.5.2

*Arguments*

* `pluginName`: string. The name of the plugin.

*Example*

```
using StringUtils;

...

b = (Plugin["StringUtils"] === StringUtils); // b is true
c = (Plugin.StringUtils === StringUtils); // c is true
```