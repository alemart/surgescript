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
        platform = currentPlatform();
        Console.print(platform);

        state = "done";
    }

    state "done"
    {
    }

    fun currentPlatform()
    {
        if(Platform.android)
            return "Android";
        else if(Platform.windows)
            return "Windows";
        else if(Platform.macos)
            return "macOS";
        else if(Platform.unix)
            return "Unix";
        else
            return "Unknown";
    }
}
```

*Available since*: Open Surge 0.6.1

Properties
----------

#### windows

`windows`: boolean, read-only.

Will be `true` if the game engine is running on Microsoft Windows.

#### unix

`unix`: boolean, read-only.

Will be `true` if the game engine is running on a Unix-like operating system such as: Linux, BSD, macOS, Android, etc.

#### macos

`macos`: boolean, read-only.

Will be `true` if the game engine is running on macOS.

#### android

`android`: boolean, read-only.

Will be `true` if the game engine is running on Android. Check [SurgeEngine.mobile](/engine/surgeengine#mobile) instead if you want to see if the game engine is running on mobile mode.
