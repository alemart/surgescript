Input
=====

The Input object is used to read input from the user.

*Example*
```cs
using SurgeEngine.Player;

object "Application"
{
    player = Player.active;

    state "main"
    {
        if(player.input.buttonDown("right"))
            Console.print("User is holding right");
    }
}
```

Factory
-------

#### Input

`Input(inputMap)`

Spawns a new Input component with the given input map.

**Note:** to read input from a specific character, prefer using [Player.input](/engine/player#input) instead.

*Arguments*

* `inputMap`: string. The name of the input map. If set to `null`, a default input map will be used.

*Returns*

An Input component.

*Example*
```cs
using SurgeEngine.Player;
using SurgeEngine.Input;

object "Application"
{
    input = Player.active.input;         // this form is preferred (used in most cases)
    //input = Input("custom input map"); // unless you need to read a custom input map

    state "main"
    {
        if(input.buttonDown("right"))
            Console.print("User is holding right");
    }
}
```

Properties
----------

#### enabled

`enabled`: boolean.

Whether the input object is enabled or not. A disabled object does not receive user input. The default value of this property is `true`.


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

#### simulateButton

`simulateButton(buttonName, down)`

Changes the input object so that `buttonName` will be identified as being held down, or not being held down, in the current frame.

*Arguments*

* `buttonName`: string. One of the following: *"up", "down", "left", "right", "fire1", "fire2", ..., "fire8"*.
* `down`: boolean. Should the button be considered as being held down?
