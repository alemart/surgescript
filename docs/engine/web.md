Web
===

Web routines.

Functions
---------

#### launchURL

`launchURL(url)`

Launches a URL using the default web browser. It's mandatory to specify a protocol. The following protocols are currently supported: *http://*, *https://*, *mailto:*

*Arguments*

* `url`: string. The URL to be launched.

*Example*
```cs
using SurgeEngine.Web;

object "Application"
{
    // Remember to change the state
    // after calling launchURL
    state "main"
    {
        Web.launchURL("http://opensurge2d.org");
        state = "done";
    }

    state "done"
    {
    }
}
```

#### encodeURIComponent

`encodeURIComponent(str)`

Encodes a component of a Universal Resource Identifier (URI). It escapes all characters of the input string, except:

```
A–Z a–z 0–9 - _ . ! ~ * ' ( )
```

*Arguments*

* `str`: string. The string to be escaped.

*Returns*

Returns the input string escaped as above.

*Example*

```cs
using SurgeEngine.Web;

// ...

user = "Jane Doe";
url = "https://my.website.name/?user=" + Web.encodeURIComponent(user);
Console.print(url);

// output:
// https://my.website.name/?user=Jane%20Doe
```

*Available since:* Open Surge 0.6.1
