Platform
========

Use `SurgeEngine.Platform` to know which platform the game engine is currently running on.

*Example*

```cs
using SurgeEngine.Platform;

object "Application"
{
    state "main"
    {
        Console.print(platformName());
        state = "done";
    }

    state "done"
    {
    }

    fun platformName()
    {
        if(Platform.isAndroid)
            return "Android";
        else if(Platform.isWindows)
            return "Windows";
        else if(Platform.isMacOS)
            return "macOS";
        else if(Platform.isUnix)
            return "Unix";
        else
            return "Unknown";
    }
}
```

*Available since*: Open Surge 0.6.1

Properties
----------

#### isWindows

`isWindows`: boolean, read-only.

Will be `true` if the game engine is running on Microsoft Windows.

#### isUnix

`isUnix`: boolean, read-only.

Will be `true` if the game engine is running on a Unix-like operating system such as: Linux, BSD, macOS, Android, etc.

#### isMacOS

`isMacOS`: boolean, read-only.

Will be `true` if the game engine is running on macOS.

#### isAndroid

`isAndroid`: boolean, read-only.

Will be `true` if the game engine is running on Android. Check [SurgeEngine.mobile](/engine/surgeengine#mobile) instead if you want to see if the game engine is running on mobile mode.
