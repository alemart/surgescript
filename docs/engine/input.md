Input
=====

The Input component is used to read input from the user.

Factory
-------

#### SurgeEngine.Input

`SurgeEngine.Input(inputMap)`

Spawns a new Input component with the given input map.

*Arguments*

* `inputMap`: string. The name of the input map (defaults to *"default"*).

*Returns*

An Input component.

*Example*
```
using SurgeEngine.Input;

object "Application"
{
    input = Input("default");

    state "main"
    {
        if(input.buttonDown("right"))
            Console.print("User is holding right");
    }
}
```

Functions
---------

#### buttonDown

`buttonDown(buttonName)`

Checks if a button of the input map is currently being held down.

*Arguments*

* `buttonName`: string. One of the following: *"up", "down", "left", "right", "fire1", "fire2", ..., "fire8"*.

*Returns*

Returns `true` if the specified button is being held down.

#### buttonPressed

`buttonPressed(buttonName)`

Checks if a button has just been pressed.

*Arguments*

* `buttonName`: string. One of the following: *"up", "down", "left", "right", "fire1", "fire2", ..., "fire8"*.

*Returns*

Returns `true` if the specified button has just been pressed.

#### buttonReleased

`buttonReleased(buttonName)`

Checks if a button has just been released.

*Arguments*

* `buttonName`: string. One of the following: *"up", "down", "left", "right", "fire1", "fire2", ..., "fire8"*.

*Returns*

Returns `true` if the specified button has just been released.