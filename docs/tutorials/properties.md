Properties
==========

In SurgeScript, [object-level variables](variables) are private. This means that these variables can only be accessed from the objects that defined them. However, SurgeScript features a *syntactic sugar* that allows objects to read and/or modify other objects' data in a way that looks like dealing with regular (public) variables. We'll call these variables **properties**.

Defining properties
-------------------

Suppose you have an object called **Animal** with a variable called *sound* and a function named *speak()*:

```
object "Animal"
{
    sound = "meow!";

    fun speak()
    {
        Console.print(sound);
    }
}
```

This object can only speak *meow!* Let's see:

```
object "Application"
{
    animal = spawn("Animal");

    state "main"
    {
        animal.speak();
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

    fun speak()
    {
        Console.print(sound);
    }
}
```

Now, external objects may access (read and write) the *sound* variable (or **property**):

```
object "Application"
{
    animal = spawn("Animal");

    state "main"
    {
        animal.sound = "woof!";
        animal.speak();
    }
}
```

Since SurgeScript 0.5.3, you may add the **readonly** specifier after the **public** keyword. Doing so disallows the modification of the property by external objects:

```
object "Animal"
{
    public readonly sound = "meow!";

    fun speak()
    {
        Console.print(sound);
    }
}

object "Application"
{
    animal = spawn("Animal");

    state "main"
    {
        //animal.sound = "woof!"; // will trigger an error
        //Console.print(animal.sound); // this is allowed
        animal.speak();
    }
}
```

Using getters and setters
-------------------------

In reality, however, there are no public variables in SurgeScript. Behind the scenes, the language defines special functions called **getters** and **setters** that will perform the read/write logic for you. Rather than using **public**, you may want to define the getters and the setters yourself:

```
object "Animal"
{
    sound = "meow!";

    fun speak()
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

This code is semantically the same; it's just a bit longer.

An advantage of defining getters and setters by yourself is that you control how the data passes through the objects. You may want to validate the data before changing the internal variables of the objects, for example.
