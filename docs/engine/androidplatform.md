AndroidPlatform
===============

Routines specific to Android. The functions below do nothing if the engine is not running on it. In order to determine if the engine is running on Android, check [Platform.isAndroid](/engine/platform#isandroid).

*Available since:* Open Surge 0.6.1

Functions
---------

#### shareText

`shareText(text)`

Share a text using the Android Sharesheet, a feature that lets the user pick which app to share data with. You may use it to let the user share a URL with a friend or on social media, for example.

*Arguments*

* `text`: string. The text to be shared.

*Example*

```cs
using SurgeEngine.Platform;
using SurgeEngine.Platform.Android;

// ...

if(Platform.isAndroid)
    Android.shareText("Download the Open Surge Engine at https://opensurge2d.org");
```
