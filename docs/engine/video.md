Video
=====

Video settings.

*Example*
```
using SurgeEngine.Video;

// display the size of the screen
object "Application"
{
    state "main"
    {
        width = Video.screenWidth;
        height = Video.screenHeight;
        Console.print(width + "x" + height);
    }
}
```

Properties
----------

#### screenWidth

`screenWidth`: number, read-only.

The width of the screen, in pixels.

#### screenHeight

`screenHeight`: number, read-only.

The height of the screen, in pixels.
