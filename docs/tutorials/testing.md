Testing your scripts
====================

There are two main ways to test your scripts:

- using the [Open Surge](#using-open-surge) game engine;
- using the [SurgeScript standalone](#using-the-standalone-version) runtime (i.e., the version without the game engine).

Using Open Surge
----------------

To test a script on Open Surge, place it on the *scripts/* folder and start the engine. Your test script must include an object called *Application*. Please remove the script after you're done with the testing.

As an example, save the following script to *scripts/hello.ss* and start the engine:
```
// hello.ss: test script
// Please remove this file after you're done
object "Application"
{
    state "main"
    {
        Console.print("Hello, world!");
        state = "blank";
    }

    state "blank"
    {
    }
}
```

Now let's begin our learning adventure: go to [Introduction to objects](objects).

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

Now let's begin our learning adventure: go to [Introduction to objects](objects).
