SurgeEngine
===========

The SurgeEngine plugin is the interface between SurgeScript (scripting language) and Open Surge (game engine).

*Example*

```
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

Open Surge version.
