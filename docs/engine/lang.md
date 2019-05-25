Lang
====

Lang is a SurgeEngine element used to read translation-specific strings, as defined in the language files (.lng).

*Example*
```
using SurgeEngine.Lang;

// Display the current language
object "Application"
{
    state "main"
    {
        currentLanguage = Lang["LANG_LANGUAGE"];
        Console.print("The current language is " + currentLanguage);
        state = "done";
    }

    state "done"
    {
    }
}
```

Functions
---------

#### get

`get(key)`

Gets the value of the specified key. Instead of calling `get()` directly, you may equivalently use the `[ ]` operator.

*Arguments*

* `key`: string. The key must always be a string.

*Returns*

The translation string associated with the specified key.
