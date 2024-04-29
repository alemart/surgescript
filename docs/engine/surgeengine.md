SurgeEngine
===========

The SurgeEngine plugin is the interface between SurgeScript (scripting language) and Open Surge (game engine).

*Example*

```cs
using SurgeEngine;

// Open Surge version
object "Application"
{
    state "main"
    {
        Console.print(SurgeEngine.version);
        state = "blank";
    }

    state "blank"
    {
    }
}
```

Properties
----------

#### version

`version`: string, read-only.

Version of the Open Surge Engine.

#### mobile

`mobile`: boolean, read-only.

Whether or not the engine has been launched in mobile mode. The user may be playing on a mobile device or on a Desktop computer with the mobile mode enabled.

*Available since:* Open Surge 0.6.1
