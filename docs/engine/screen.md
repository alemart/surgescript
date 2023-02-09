Screen
======

Screen settings.

*Example*
```cs
// display the size of the screen
using SurgeEngine.Video.Screen;

object "Application"
{
    width = Screen.width;
    height = Screen.height;

    state "main"
    {
        Console.print(width + "x" + height);
        state = "done";
    }

    state "done"
    {
    }
}
```

Properties
----------

#### width

`width`: number, read-only.

The width of the screen, in pixels.

#### height

`height`: number, read-only.

The height of the screen, in pixels.
