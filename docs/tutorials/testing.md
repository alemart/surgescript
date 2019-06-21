Testing your scripts
====================

There are two main ways to test your scripts:

- using the [Open Surge](#using-open-surge) game engine;
- using the [SurgeScript standalone runtime](#using-the-standalone-version) (i.e., the version without a game engine).

Using Open Surge
----------------

To test a script on Open Surge, place it on the *scripts/* folder and start the engine. Your test script must include an object called *Application*. Make sure to remove the script after you're done with the testing.

As an example, save the following script to *scripts/hello.ss* and start the engine:
```
// hello.ss: test script
// Please remove this file after you're done
object "Application"
{
    state "main"
    {
        Console.print("Hello, world!");
        state = "done";
    }

    state "done"
    {
    }
}
```

You should see the *Hello, world!* message as a result.

**Linux users:** when using a system-wide installation, you may place your scripts on *~/.local/share/opensurge2d/opensurge/scripts/* (i.e., *$XDG_DATA_HOME/opensurge2d/opensurge/scripts/*).

Ready to proceed? Let's go to [Introduction to objects](/tutorials/objects)!

Using the standalone version
----------------------------

If you've downloaded the standalone version of the language:

- First of all, save the following script to a file named *hello.ss*. For testing purposes, please place the file on the same directory as the surgescript executable.
```
// hello.ss: test script
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

To begin our learning adventure, let's go to [Introduction to objects](/tutorials/objects).
