SurgeScript: a scripting language for games
===========================================

<img src="docs/img/surge.png" alt="Surge" align="right" width="384">

Unleash your creativity!
------------------------

SurgeScript is a scripting language for games. It empowers you to unleash your creativity and build your own interactive stuff with ease and power. It's such a joy to use SurgeScript! You will love it!

**First-time here?**<br>
Please go to the [Welcome page](https://alemart.github.io/surgescript).

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

Please read the [SurgeScript Crash Course](https://alemart.github.io/surgescript/tutorials/hello) to learn more. For more examples, please check the [examples](examples) folder.

FAQ
---

##### What is SurgeScript?

SurgeScript is a scripting language for games.

##### Who created it?

SurgeScript has been originally created by [Alexandre Martins](https://github.com/alemart), a computer scientist from Brazil. He has also created the [Open Surge game engine](http://opensurge2d.org), hence the name *SurgeScript*.

##### How do I learn it?

Please read the [documentation](https://alemart.github.io/surgescript/). The developer also releases SurgeScript tutorials on his [YouTube channel](https://youtube.com/alemart88).

##### Why use SurgeScript?

Are you a game developer in pain? Are you a modder feeling confined?

- Unleash your creativity! — build your own content with ease and power!

Unlike other programming languages, SurgeScript has been designed with the specific needs of games in mind. Its features include:

- The state-machine pattern: objects are state machines (and the game loop is defined implicitly)
- The composition approach: you may design complex objects and behaviors by means of composition
- The hierarchy system: objects have a parent and may have children, in a tree-like structure

Other benefits include:

- Small footprint, easy to integrate
- C-like syntax, including plenty of features
- Automatic garbage collection
- Easy to extend via Plugins
- It's free software

These features come from the experience of the developer dealing with game engines, applications related to computer graphics and so on. Some of the best practices have been incorporated into the language itself, making things really easy for developers and modders.

##### How do I embed SurgeScript into my project?

If you're a C developer, you may embed SurgeScript into your project studying the *main.c* file (located in the *src/* folder). The steps are:

1. Create a SurgeScript Virtual Machine (VM).
2. Insert the scripts you want into the VM (let it compile the scripts).
3. Launch the VM.
4. In your game loop, update the VM.
5. Once you're done, release the VM.

You'll need to `#include <surgescript.h>` in your code and link your project with *-lsurgescript -lm*.

Additionally, you may call C code from SurgeScript via *binding*. See *src/surgescript/runtime/sslib/* for more information.

SurgeScript is available as a library.

##### How do I compile SurgeScript?

If you're using [Open Surge](http://opensurge2d.org), you don't need to compile SurgeScript. It's compiled for you.

If you want to compile SurgeScript by yourself, you'll need [CMake](https://cmake.org) and gcc.

First of all, please download the source code:

```
git clone https://github.com/alemart/surgescript.git
cd surgescript
```

Then, you may compile SurgeScript as follows:

```
mkdir build && cd build
cmake ..
make
```

A **surgescript** executable will be available on the project folder for testing scripts. Additionally, SurgeScript will also be compiled as a static library. To make a system-wide installation, run:

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
