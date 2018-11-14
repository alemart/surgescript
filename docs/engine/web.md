Web
===

Web routines.

Functions
---------

#### launchURL

`launchURL(url)`

Launches a URL using the default web browser. It's mandatory to specify a protocol. The following protocols are currently supported: *http://*, *https://*, *mailto:*.

*Arguments*

* `url`: string. The URL to be launched.

*Example*
```
using SurgeEngine.Web;

object "Application"
{
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
