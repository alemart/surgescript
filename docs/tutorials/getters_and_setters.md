Getters and setters
===================

In SurgeScript, variables are private. This means that variables may only be accessed from the objects that defined them. However, SurgeScript features a *syntactic sugar* that allows objects to modify other objects' data in a way that looks like dealing with regular (public) variables.

Suppose you have an object called **Animal** with a variable called *sound* and a function called *talk()*:

```
object "Animal"
{
    sound = "meow!";

    fun talk()
    {
        Console.print(sound);
    }
}
```

This object can only say *meow!* Let's see:

```
object "Application"
{
    animal = spawn("Animal");

    state "main"
    {
        animal.talk();
    }
}
```

Run this script and you'll see:

```
meow!
meow!
meow!
meow!
...
```

What if an external object could modify the sound of the animal? Trying to access `animal.sound` externally will trigger an error, unless you add the **public** keyword to your variable:

```
object "Animal"
{
    public sound = "meow!";

    fun talk()
    {
        Console.print(sound);
    }
}
```

Now, external objects may access (read and write) the *sound* variable:

```
object "Application"
{
    animal = spawn("Animal");

    state "main"
    {
        animal.sound = 'rrrrgh!!!';
        animal.talk();
    }
}
```

Since SurgeScript 0.5.3, you may add the **readonly** specifier after the **public** keyword. Doing so disallows the modification of the variable by external objects:

```
object "Animal"
{
    public readonly sound = "meow!";

    fun talk()
    {
        Console.print(sound);
    }
}

object "Application"
{
    animal = spawn("Animal");

    state "main"
    {
        //animal.sound = 'rrrrgh!!!'; // will trigger an error
        animal.talk();
    }
}
```

In reality, however, there are no public variables in SurgeScript. Behind the scenes, the language defines special functions called **getters and setters** that will perform the read/write logic for you. Rather than typing public, you may want to define the getters and the setters yourself:

```
object "Animal"
{
    sound = "meow!";

    fun talk()
    {
        Console.print(sound);
    }

    fun set_sound(value)
    {
        sound = value;
    }

    fun get_sound()
    {
        return sound;
    }
}
```

This code is the same (semantically); it's just a bit longer.

An advantage of defining getters and setters yourself is that you control how the data passes through the objects. You may want to validate the data before changing the internal variables of the objects, for example.
