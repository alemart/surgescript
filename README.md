## SurgeScript
SurgeScript is a lightweight programming language for computer games and interactive apps, built originally for the Open Surge Engine.

### Introduction
Computer games usually have tons of objects running around the screen. Some of them enable users (aka "modders") to modify the content and the behavior of the games (creating MODs). However, adding modding capabilities to a game engine usually imply a lot of work to developers.

SurgeScript enables developers to easily add modding capabilities to their programs, so that users can customize the behavior of the game objects. SurgeScript is lightweight and simple to use. Having a C-like syntax, it's suitable to experts and novices alike.

### Scenario

So, you have a game or an interactive app with 3 objects on the screen: a ball and two rackets. You'd like to let users customize their behavior, so that you don't end up with boring pong. Users can come up with unlimited creativity: the ball can move in any way they like, maybe there should be more rackets, or, who knows, a change of scenario would be interesting! Well, users can easily make those changes to the original game/app.

In SurgeScript, objects are defined in scripts. Scripts are text files that encode how objects are going to behave.

Objects hide their inner workings from the outer world, so that nobody needs to know exactly *how* they work. Everybody else just need to know *what* they do. They expose *public functions* that enables the world to interact with each of them. In order words, objects talk to each other using *API calls*.

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

### Examples

The following is an example script:

TODO

*pong.ss*
```
// This object represents our game/app
object Application
{
    // TODO
}

object Racket 
{
    // ...
}

object Ball
{
    // ...
}
```

### Using it in C

TODO

### Compiling
In a terminal, type:
```
$ make
```

© 2016  Alexandre Martins <alemartf(at)gmail(dot)com>
