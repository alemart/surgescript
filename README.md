SurgeScript: a scripting language for games
===========================================

<img src="docs/img/surge.png" alt="Surge" align="right" width="384">

Unleash your creativity!
------------------------

SurgeScript is a scripting language for games. Use it to unleash your creativity and build your own amazing interactive content! It's such a joy to use SurgeScript! You will love it!

**First time here?**<br>
Go to the [Welcome page](https://alemart.github.io/surgescript).

**Where to get it?**<br>
Get it on the [Download page](https://alemart.github.io/surgescript/download).

**Need help?**<br>
Feel free to [contact the developer](https://github.com/alemart).

The 15-second example
---------------------

The following script prints a message to the screen:

```
// My first application
object "Application"
{
    state "main"
    {
        Console.print("Hello, world!");
        Application.exit();
    }
}
```

To test this script, save it to *hello.ss* and run:

```
surgescript hello.ss
```

You'll see:

```
Hello, world!
```

FAQ
---

##### What is SurgeScript?

SurgeScript is a scripting language for games. It lets you unleash your creativity and build your own amazing interactive content!

##### How do I learn SurgeScript?

Go to the [SurgeScript Crash Course](https://alemart.github.io/surgescript/)! Also take a look on the [video tutorials](https://youtube.com/alemart88) and on the [examples](examples).

##### Why use SurgeScript?

Unlike other programming languages, SurgeScript has been designed with the specific needs of games in mind. Its features include:

- The state-machine pattern: objects are state machines, making it easy to create in-game entities
- The composition approach: you may design complex objects and behaviors by means of composition
- The hierarchy system: objects have a parent and may have children, in a tree-like structure
- The game loop: it's defined implicitly
- Automatic garbage collection, object tagging and more!

SurgeScript is meant to be used in games and in interactive applications. It's easy to integrate it into existing code, it's easy to extend, it features a C-like syntax, and it's free and open-source software.

SurgeScript has been designed based on the experience of its developer dealing with game engines, applications related to computer graphics and so on. Some of the best practices have been incorporated into the language itself, making things really easy for developers and modders.

##### Who created SurgeScript?

SurgeScript has been created by [Alexandre Martins](https://github.com/alemart), a computer scientist from Brazil. He has also created the [Open Surge game engine](http://opensurge2d.org), hence the name SurgeScript.

##### How do I compile SurgeScript?

If you're using [Open Surge](http://opensurge2d.org), you don't need to compile SurgeScript. It's compiled for you.

If you want to compile SurgeScript by yourself, you'll need a C compiler and [CMake](https://cmake.org).

First of all, please download the source code:

```
git clone https://github.com/alemart/surgescript.git
cd surgescript
```

You may compile SurgeScript as follows:

```
mkdir build && cd build
cmake ..
make
```

A **surgescript** executable will be available on the project folder for testing scripts. Additionally, SurgeScript will also be compiled as a static library. To perform a system-wide install, run:

```
sudo make install
```

**Linux users:** the installation folder defaults to */usr/local*. If you want to change it (optional), call `cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install` before running make.

##### How do I build the documentation?

You'll need [mkdocs](http://www.mkdocs.org). After you get the sources, go to the project folder and run:

```
mkdocs build
```

The documentation will be available in the *site/* subdirectory.

##### How do I embed SurgeScript into my project?

SurgeScript is available as a library. If you're a C developer, you may embed SurgeScript into your project by studying the *main.c* file (located in the *src/* folder). The steps are:

1. Create a SurgeScript Virtual Machine (VM).
2. Insert the scripts you want into the VM (let it compile the scripts).
3. Launch the VM.
4. In your game loop, update the VM.
5. Once you're done, release the VM.

You'll need to `#include <surgescript.h>` in your code and link your project with *-lsurgescript -lm*.

Additionally, you may call C code from SurgeScript via *binding*. See *src/surgescript/runtime/sslib/* for more information.

