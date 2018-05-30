Plugins
=======

Plugins are objects that allow you to extend the functionalities of the language. Once declared, they can be imported anywhere in the code. They can also be easily shared with others, so everyone benefits.

*Available since:* SurgeScript 0.5.2

Basic example
-------------

Suppose you want to extend the language by adding new routines related to string manipulation. The routines you want will be used frequently throughout your project, and you'd like easy access to them. This makes it perfect for implementing them as a plugin.

We'll define an object called `StringUtils` with a `reverse()` function that reverses a string. We make the object a plugin by annotating it with `@Plugin`. Please see in the example below:

```
// File: stringutils.ss

@Plugin
object "StringUtils"
{
    fun reverse(str)
    {
        buf = "";
        for(i = str.length - 1; i >= 0; i--)
            buf += str[i];
        return buf;
    }
}
```

When you annotate an object definition with `@Plugin`, SurgeScript will spawn an instance of such an object before spawning the `Application`. Additionally, the plugin object may be imported anywhere with the `using` keyword:

```
// File: app.ss
using StringUtils;

object "Application"
{
    state "main"
    {
        str = StringUtils.reverse("alucard");
        Console.print(str); // will print "dracula"
        exit();
    }
}
```

**Note:** it's recommended not to include any special characters in the name of your plugins, otherwise you may have trouble importing them.

In the example above, `Application` and `StringUtils` are located in two separate files. If you want to test them using the SurgeScript command line utility, pass both files as parameters:

```
surgescript plugin.ss app.ss
```

> **Spawning order**
>
> Although plugins are spawned before the *Application* object, for the time being they are spawned in no particular order. You may not access a plugin from within the constructor of another plugin, as of now there is no dependency resolver implemented. Use the *main* state.

Package example
---------------

You may use the Plugin system to create an independent package (or namespace) that aggregates many functionalities. This lets you separate your code in coherent, logical units and also helps you to distribute it.

Suppose that user *Parrot*, the owner of *ParrotSoft*, created SurgeScript utilities related to string manipulation, matrix math and complex numbers. He decided to share his code as a single package called `ParrotSoft`. Since he implemented getters for each of his utilities (see the code below), his package provides easy access to all of them. Furthermore, he annotated his package with `@Plugin` (but **not** the individual utilities), so his package can be accessed anywhere.

```
// File: parrotsoft.ss

@Plugin
object "ParrotSoft"
{
    matrices = spawn("ParrotMatrices");
    complex = spawn("ParrotComplexNumbers");
    stringUtils = spawn("ParrotStringUtils");

    fun get_Matrices()
    {
        return matrices;
    }

    fun get_Complex()
    {
        return complex;
    }

    fun get_StringUtils()
    {
        return stringUtils;
    }
}

// ...
```

A user of the `ParrotSoft` package may import *Parrot*'s utilities simply by importing the plugin. Individual utilities may also be imported. The code below shows a usage example:

```
using ParrotSoft.Matrices.Matrix2;
using ParrotSoft.Complex;
using ParrotSoft;

object "Application"
{
    state "main"
    {
        identity = Matrix2([[1, 0], [0, 1]]); // 2x2 matrix
        z = Complex(2, 3); // 2 + 3i
        neon = ParrotSoft.StringUtils.reverse("noen");
        exit();
    }
}
```

In the example above, writing `Complex` is equivalent to writing `ParrotSoft.Complex`, since we imported this utility with `using ParrotSoft.Complex`. Notice that `ParrotSoft` implements a getter for `Complex`. Additionally, `ParrotSoft` has been annotated with `@Plugin`, but `Complex` has **not**. This means that the statement `using Complex` will **not** yield correct results - and it shouldn't, since `Complex` is a part of the `ParrotSoft` package that can only be accessed from it. Finally, since we haven't imported `StringUtils` explicitly, we type `ParrotSoft.StringUtils` to access it.