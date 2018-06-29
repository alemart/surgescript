SurgeEngine
===========

The SurgeEngine plugin is the interface between SurgeScript (scripting language) and Open Surge (game engine).

Example
-------
```
// Display Open Surge version
using SurgeEngine;

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

* `version`: string, read-only. Open Surge version.
