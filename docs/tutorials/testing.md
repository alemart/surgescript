Testing your scripts
====================

There are two main ways of testing your scripts: using the Open Surge Engine or the [standalone version](https://github.com/alemart/surgescript) of SurgeScript (i.e., the version without the game engine).

Testing on Open Surge
---------------------

TODO

Using the standalone version
----------------------------

If you've downloaded the standalone version of the language:

- First of all, save the following script to a file named *hello.ss*. This file should be placed on the same directory as the surgescript executable.
```
object "Application"
{
    state "main"
    {
        Console.print("Hello, world!");
        Application.exit();
    }
}
```
- Then, open up a Terminal and type:
```
cd /path/to/surgescript
./surgescript hello.ss
```
- If you're using Microsoft Windows, open up a Command Prompt and type:
```
cd C:\path\to\surgescript
surgescript.exe hello.ss
```
- You should see the output of the script as a result:
```
Hello, world!
```