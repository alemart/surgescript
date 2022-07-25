Text
====

The Text object allows you to display custom texts in the game. The parent object is required to be an [entity](/engine/entity).

Factory
-------

#### Text

`UI.Text(font)`

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
    text = Text("GoodNeighbors");
    transform = Transform();

    state "main"
    {
        // position the text
        player = Player.active;
        transform.position = player.transform.position;
        transform.translateBy(0, -50);

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

#### size

`size`: [Vector2](/engine/vector2) object, read-only.

The size, in pixels, of the rendered text.

*Available since:* Open Surge 0.5.1

#### align

`align`: string.

The alignment of the text. One of the following: *"left"*, *"center"*, *"right"*.

#### visible

`visible`: boolean.

Is the Text object visible?

#### maxLength

`maxLength`: number.

The maximum number of characters to be displayed, ignoring *<color>* tags and spaces.

*Available since:* Open Surge 0.5.1

#### maxWidth

`maxWidth`: number.

The maximum width of the text, in pixels. Setting this value will enable wordwrap.

#### zindex

`zindex`: number.

The zindex of the Text object. Defaults to 0.5.

#### offset

`offset`: [Vector2](/engine/vector2) object.

An *(x,y)* offset relative to the parent object. Defaults to zero.
