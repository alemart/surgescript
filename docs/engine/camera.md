Camera
======

The Camera object can be used to control what content (in world space) is rendered to the screen. The Camera is represented by a 2-dimensional point in world space that is mapped to the center of the screen.

> **HUD elements**
>
> Objects tagged as *"detached"* do not follow the camera. They are not in world space, but in screen space. This is useful for creating HUD elements.

*Example*

```
// Import the Camera object
using SurgeEngine.Camera;

// Reading the Camera status
// This app prints the position of the camera at every second
object "Application"
{
    state "main"
    {
        if(timeout(1))
            state = "print";
    }

    state "print"
    {
        Console.print(Camera.xpos + "," + Camera.ypos);
        state = "main";
    }
}
```




Properties
----------

#### xpos

`xpos`: number.

The x-position of the camera in world space.

#### ypos

`ypos`: number.

The y-position of the camera in world space.
