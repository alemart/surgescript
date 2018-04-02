Components
==========

Entity Component System (ECS) is a software pattern used in game development. The idea is that in-game objects (called *entities*) can be customized by adding or removing objects (called *components*) during runtime. components add functionalities or behaviors to the entities to which they are attached to. In SurgeScript, both entities and components are objects - the difference is conceptual.

As an example, consider an in-game object called **Parrot**. Objects by themselves are empty; they do nothing. However, an interesting behavior of parrots is that they are blabbers: they keep repeating the same thing over and over again. So, let's create an empty object **Parrot** and spawn on it the **Blabber** component:

```
object "Parrot"
{
    blabber = spawn("Blabber");

    state "main"
    {
    }
}

object "Blabber"
{
    state "main"
    {
        if(timeout(2)) // blab every 2 seconds
            state = "blab";
    }

    state "blab"
    {
        Console.print("Hello!");
        state = "main";
    }
}
```

Now, whenever you spawn a Parrot, it will keep blabbing unceasingly:

```
Hello!
Hello!
Hello!
...
```

Suppose now that your Parrot is a Kamikaze Robot: it will blab so much that it will explode itself after some time. We'll attach a **Time Bomb** to the parroting robot, so that it will stop blabbing after a while:

```
object "Time Bomb"
{
    state "main"
    {
        if(timeout(15)) // explode after 15 seconds
            state = "explode";
    }

    state "explode"
    {
        Console.print("BOOOM!");
        parent.destroy(); // destroy the parent object
    }
}

object "Parrot"
{
    blabber = spawn("Blabber");
    bomb = spawn("Time Bomb");

    state "main"
    {
    }
}
```

Now, here's the result:

```
Hello!
Hello!
Hello!
Hello!
Hello!
Hello!
Hello!
BOOOM!
```

In this example, **Parrot** is an entity and **Blabber** and **Time Bomb** are its components.

#### Design considerations

In some implementations of ECS, entities are usually implemented as integer numbers and components consist of raw data only. The game logic is executed by the System layer, which iterates throughout all entities continuously and executes the logic according to the components attached to them.

In SurgeScript, however, components execute their own code and can be attached to entities simply by spawning them as children.

#### Composition over inheritance

SurgeScript does not support inheritance, a feature found in many object-oriented languages. The usage of components is encouraged instead. SurgeScript favors composition over inheritance, allowing for greater flexibility when defining in-game objects.