Text
====

The Text object allows you to display custom texts in the game. The parent object is required to be an [entity](entity).

Factory
-------

#### Text

`SurgeEngine.UI.Text(font | null)`

Spawns a new Text object with the given font name. If `null` is provided as the font name, then a default font will be used.

*Arguments*

* `font`: string. The name of a font (defined in the *fonts/* folder).

*Returns*

A Text object.

*Example*
```
using SurgeEngine.UI.Text;
using SurgeEngine.Transform;
using SurgeEngine.Player;

// Place this on your level to display
// the name of the player
object "PlayerName" is "entity", "awake"
{
    text = Text(null);
    transform = Transform();

    state "main"
    {
        // position the text
        player = Player.active;
        transform.worldX = player.transform.worldX;
        transform.worldY = player.transform.worldY - 50;

        // configure the text
        text.align = "center";
        text.text = player.name;
    }
}
```



Properties
----------

#### text

`text`: string.

The text to be displayed.

#### font

`font`: string, read-only.

The name of the font in use.

#### align

`align`: string.

The alignment of the text. One of the following: *"left"*, *"center"*, *"right"*.

#### visible

`visible`: boolean.

Is the Text object visible?

#### width

`width`: number.

The (maximum) width of the text, in pixels. Setting this value will enable wordwrap.

#### zindex

`zindex`: number.

The zindex of the Text object. Defaults to 0.5.

#### transform

`transform`: [Transform](transform) object, read-only.

The Transform of the Text object.
