Getters and setters
===================

There are no public variables in SurgeScript. This means that variables may only be accessed from the objects that defined them. However, SurgeScript has a *syntax sugar* that allows objects to modify other objects' data in a way that looks like dealing with regular variables.

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
```

What if an external object could modify the sound of the animal? Trying to access `animal.sound` externally will trigger an error, unless we define getters and setters:

```
object "Animal"
{
    sound = "meow!";

    fun talk()
    {
        Console.print(sound);
    }

    fun setSound(value)
    {
        sound = value;
    }

    fun getSound()
    {
        return sound;
    }
}
```

Now, external objects may modify the *sound* variable. In reality, they are calling *setSound()* and *getSound()* behind the scenes, but the syntax sugar makes it much neater.

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

An advantage of defining getters and setters is that you control what goes inside the objects. You may want to validate the data before changing the internal variables of the objects, for example.