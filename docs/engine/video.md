Video
=====

Video settings.

*Example*

```cs
// display the current Video mode
using SurgeEngine.Video;

object "Application"
{
    state "main"
    {
        Console.print(Video.mode);
        state = "done";
    }

    state "done"
    {
    }
}
```

Properties
----------

#### mode

`mode`: string.

The current video mode. The following are the valid values:

* `"default"`: the size of the [screen](#screen) is set to its default.
* `"fill"`: the size of the screen is set to the size of the window.
* `"best-fit"`: similar to `"fill"`, except that the aspect ratio of the default size of the screen is maintained.

*Available since:* Open Surge 0.6.1

#### Screen

`Screen`: [Screen](/engine/screen) object, read-only.

Screen settings.
