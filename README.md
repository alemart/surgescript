## SurgeScript
SurgeScript is a scripting language for games. It is easy and fun to use, yet powerful enough for experts. It may be embedded into any C project.

> **First timer?**
>
> If this is your first time here, please go to the [Introduction](https://alemart.github.io/surgescript) page.

**© 2016-2018  Alexandre Martins &lt;alemartf(at)gmail(dot)com&gt;**

### Introduction
Computer games usually have tons of objects running around the screen. Some of them enable users (aka "modders") to modify the content and the behavior of the games (creating MODs). However, adding modding capabilities to a game engine usually imply a lot of work to developers.

SurgeScript enables developers to easily add modding capabilities to their programs, so that users can customize the behavior of the game objects. SurgeScript is lightweight and simple to use. Having a C-like syntax, it's suitable to experts and novices alike.

**Main features**:

- **Object oriented programming language**: SurgeScript allows users to write code for discrete entities called "objects". Unlike other languages on the market, SurgeScript embeds a state machine within the objects. This is very helpful for game development, making things really easy for developers.
- **Component-based approach**: users can create complex objects and behaviors by means of composition. While one object may describe a physical entity on your game, another may describe a behavior that can be attached to game objects. Users may compose those. Unlike other languages on the market, there is no inheritance. SurgeScript favors composition over inheritance.
- **Object design**: objects define a set of variables, states and functions. Variables may be modified from within the objects themselves. Only one state may be active at any given time. Functions are exposed to the outer world, and any object may call exposed functions.
- **Objects do not mess with each others' internals**: this is a golden rule. In order to promote low coupling and high cohesion, SurgeScript asks users to define an API for each object. The internals of an object may only be modified through API calls, or from the objects themselves.
- **Built-in hierarchy**: unlike other languages on the market, SurgeScript features a built-in parent-child hierarchy for the objects. Objects may be created at anytime (during runtime) and are subject to this hierarchy. This is a know pattern on the computer graphics world and it is very useful for developing games and creating interactive apps.
- **Type system**: SurgeScript is a dynamically typed language. Five basic types are available: string, number, boolean, object and null.
- **C-like syntax**: constructions like if, while, variable assignments, function calls and so on are all available.
- **Automatic garbage collection**: unneeded (unreachable) objects are automatically discarded from memory.

### Quick start

Open a terminal and type:
```
$ git clone https://github.com/alemart/surgescript.git
$ cd surgescript
$ make
$ ./surgescript examples/hello.ss
```

You should see:
```
Hello, world!
```

### Examples

This is an example script:

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

It first prints a message to the console. Then, it exits the app.

For more examples, please check the [examples](./examples) folder.

### Mini-FAQ

##### What is SurgeScript?

SurgeScript is a scripting language designed for games.

##### Who created it?

SurgeScript has been originally created by Alexandre Martins, a developer from Brazil. The author has also created the Open Surge game engine, thus the name *SurgeScript*.

##### How do I learn SurgeScript?

Read the [Documentation](https://alemart.github.io/surgescript/).

##### Why not use some other languages such as Lua?

Unlike other programming languages, SurgeScript has been designed with the specific needs of games in mind. This includes: the state-machine pattern (all objects are state machines), the composition-approach (you may design complex objects and behaviors by means of composition), as well as the hierarchy system (objects have a parent and may have children, in a tree-like structure). This comes from years of experience of the author dealing with game engines, applications related to computer graphics, and so on. Some of the best practices have been incorporated into the language itself, making things really easy for developers and modders.

##### How do I embed SurgeScript into my game?

TODO

### App Structure

SurgeScript comes with a VM, used to run scripts. Scripts encode objects. Objects have an hierarchy between them. For example:

```
               Application
               /         \
              /           \
          Level           Math
          /    \            \
         /      \            \
LevelObject1   LevelObject2   ...
```

At the top of the hierarchy, there's the **Application**. It's the *root object*, and it commands some basic functionalities of the app (for example, when to quit - everything's customizable).

Below the Application, there are objects created by the Application itself (via script): **Level** and **Math** (for example). These are called *children* objects. The Application is their *parent*. We say that the Application has *spawned* the Level and Math. Similarly, we say that Level has *spawned* the level objects.

This hiearchy comes with a curious feature: whenever an object is destroyed, so are its children. So, if you delete the Level, all of its memory is automatically released. Similarly, if you delete the Application, the SurgeScript VM quits.

### Scenario

Imagine you have a game or an interactive app with 3 objects on the screen: a ball and two rackets. You'd like to let users customize their behavior, so that you don't end up with boring pong. Users can come up with unlimited creativity: the ball can move in any way they like, maybe there should be more rackets, or, who knows, a change of scenario would be interesting! Well, users can easily make those changes to the original game/app.

In SurgeScript, objects are defined in scripts. Scripts are text files that encode how objects are going to behave.

Objects hide their inner workings from the outer world, so that nobody needs to know exactly *how* they work. Everybody else just need to know *what* they do. They expose *public functions* that enables the world to interact with each of them. In order words, objects talk to each other using *API calls*.

### Using it in C

TODO

### Compiling
In a terminal, type:
```
$ make
```
You'll need a C compiler such as gcc, as well as make.