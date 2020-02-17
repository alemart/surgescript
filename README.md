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

To test the script, save it to *hello.ss* and run:

```
surgescript /path/to/hello.ss
```

FAQ
---

##### What is SurgeScript?

SurgeScript is a scripting language for games. It lets you unleash your creativity and build your own amazing interactive content!

##### How do I learn SurgeScript?

Go to the [SurgeScript Crash Course](https://alemart.github.io/surgescript/). Also take a look at the [video tutorials](https://youtube.com/alemart88) and check the [examples](examples).

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

First of all, get the source code and extract the package. Next, compile SurgeScript as follows:

```
cd /path/to/sources
mkdir build && cd build
cmake ..
make
```

A **surgescript** executable will be available in the project folder for testing scripts. Additionally, SurgeScript will also be compiled as a library. To perform a system-wide install, run:

```
sudo make install
```

**Linux users:** the installation directory defaults to */usr*. You may change it by calling `cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install` before `make`.

##### How do I build the documentation?

You'll need [mkdocs](http://www.mkdocs.org). After you get the sources, go to the project folder and run:

```
mkdocs build
```

The documentation will be available in the *site/* subdirectory.

##### How do I embed SurgeScript into my project?

SurgeScript is available as a library. If you're a C/C++ developer, you may embed SurgeScript into your project by studying file *main.c* (located in *src/*). The steps are as follows:

1. Create a SurgeScript Virtual Machine (VM).
2. Insert the scripts you want into the VM (let it compile the scripts).
3. Launch the VM.
4. In your game loop, update the VM.
5. Once you're done, release the VM.

You need to `#include <surgescript.h>` in your code and link your project with `-lsurgescript`.

Additionally, you may call C/C++ code from SurgeScript via *binding*. Explore *src/surgescript/runtime/sslib/* for more information.

**Tip:** to print the command-line options required to link your project with SurgeScript, run:

```
pkg-config --cflags --libs surgescript
```

If you prefer static linkage, run:

```
pkg-config --cflags --libs --static surgescript-static
```

These may be combined with command substitution:

```
gcc example.c -o example $(pkg-config --cflags --libs surgescript)
```