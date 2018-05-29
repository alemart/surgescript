SurgeScript
===========

![SurgeScript](docs/img/opensurge.png)

SurgeScript is a scripting language for games. It has unique features that make it suitable for games - and it is extremely valuable for empowering users to generate their own content. SurgeScript is built in C, which is nearly universal.

**First-timer?**

If this is your first time here, please go to the [Welcome page](https://alemart.github.io/surgescript).

Get SurgeScript
---------------

Get SurgeScript on the [Download page](https://alemart.github.io/surgescript/download).

Already using SurgeScript? Please support us, so we can continue working on this project and on Open Surge:

[![Support us](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=3WAZYYTB22KFG)

Example
-------

The following script prints a message to the screen:

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

To test this script, save it to *hello.ss* and run:

```
surgescript hello.ss
```

You'll see:

```
Hello, world!
```

Please read the [SurgeScript Crash Course](https://alemart.github.io/surgescript/tutorials/hello) to learn more.

For more examples, please check the [examples](examples) folder.

Compiling SurgeScript
---------------------

If you're using Open Surge, you don't need to compile SurgeScript. It's compiled for you.

If you want to compile SurgeScript by yourself, you'll need [CMake](https://cmake.org) and gcc.

First of all, download the source code:

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

FAQ
---

##### What is SurgeScript?

SurgeScript is a scripting language for games.

##### Who created it?

SurgeScript has been originally created by [Alexandre Martins](https://github.com/alemart), a developer from Brazil. He has also created the Open Surge game engine, thus the name *SurgeScript*.

##### How do I learn SurgeScript?

Please read the [documentation](https://alemart.github.io/surgescript/). The author also releases SurgeScript tutorials on his [YouTube channel](https://youtube.com/alemart88).

##### Why not use other languages?

Unlike other programming languages, SurgeScript has been designed with the specific needs of games in mind. Its features include:
- The state-machine pattern: objects are state machines (and the game loop is defined implicitly)
- The composition approach: you may design complex objects and behaviors by means of composition
- The hierarchy system: objects have a parent and may have children, in a tree-like structure
These features come from the experience of the author dealing with game engines, applications related to computer graphics and so on. Some of the best practices have been incorporated into the language itself, making things really easy for developers and modders.

##### How do I embed SurgeScript into my project?

If you're a C developer, you may embed SurgeScript into your project studying the *main.c* file (located in the *src/* folder). The steps are:

1. Create a SurgeScript Virtual Machine (VM).
2. Insert the scripts you want into the VM (let it compile the scripts).
3. Launch the VM.
4. At every frame of your game/app, update the VM (game loop).
5. Once you're done, release the VM.

You'll need to `#include <surgescript.h>` in your code and link your project with *-lsurgescript -lm*.

Additionally, you may call C code from SurgeScript via *binding*. See *src/surgescript/runtime/sslib/* for more information.

##### How do I build the documentation?

You'll need [mkdocs](http://www.mkdocs.org). After you get the sources, go to the project folder and run:

```
mkdocs build
```

The documentation will be available in the *site/* subdirectory.
