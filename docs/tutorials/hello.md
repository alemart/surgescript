Hello, World!
=============

SurgeScript creates automatically an object called **Application**. That object is defined by the user. Let's see how we can use that to make a simple program:

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

In SurgeScript, each object defines a state machine (more on that later). The initial state is called **main**. In the code just presented, we display a *Hello, world!* message and then exit the app.

Now let's [test your scripts](testing).
