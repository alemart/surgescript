Packages
========

Introduction
------------

Packages are objects that allow you to extend the functionalities of the language. Once declared, they can be imported anywhere in the code. They can also be easily shared with others, so everyone benefits.

*Available since:* SurgeScript 0.5.2

Basic example
-------------

Suppose you want to extend the language by adding new routines related to string manipulation. These routines will be used frequently throughout your project, and you'd like easy access to them. This makes it perfect for implementing them as a package.

We'll define an object called `StringUtils` with a `reverse()` function that reverses a string. We make the object a package by annotating it with `@Package`. Take a look at the example below:

```cs
// File: stringutils.ss

@Package
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

When you annotate an object definition with `@Package`, SurgeScript will spawn an instance of such an object before spawning the `Application`. Additionally, the package object may be imported anywhere with the `using` keyword:

```cs
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

**Note:** it's recommended not to include any special characters in the name of your packages.

In the example above, `Application` and `StringUtils` are located in two separate files. If you want to test them using the SurgeScript command line utility, pass both files as parameters:

```sh
surgescript package.ss app.ss
```

> **Spawning order**
>
> Although packages are spawned before the *Application* object, for the time being they are spawned in no particular order. You may not access a package from within the constructor of another package, as of now there is no dependency resolver implemented. Use the *main* state.

**Note:** before SurgeScript 0.5.4, you would annotate the object with `@Plugin` instead.

Package example
---------------

Packages may aggregate many code units with different functionalities. This lets you separate your code in coherent, logical units and also helps you to distribute it.

Suppose that user *Parrot*, the owner of *ParrotSoft*, created SurgeScript utilities related to string manipulation, matrix math and complex numbers. He decided to share his code as a single package called `ParrotSoft`. Since he implemented [getters](/tutorials/properties) for each of his utilities, his package provides easy access to all of them. Furthermore, he annotated his package with `@Package` (but **not** the individual utilities), so his package can be accessed anywhere.

```cs
// File: parrotsoft.ss

@Package
object "ParrotSoft"
{
    public readonly Matrices = spawn("Parrot Matrices");
    public readonly Complex = spawn("Parrot Complex");
    public readonly StringUtils = spawn("Parrot StringUtils");
}
```

A user of the `ParrotSoft` package may import *Parrot*'s utilities simply by importing the package. Individual utilities may also be imported. The code below shows an example:

```cs
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

In the example above, writing `Complex` is equivalent to writing `ParrotSoft.Complex`, since we imported this utility with `using ParrotSoft.Complex`. Notice that `ParrotSoft` implements a getter for `Complex`. Additionally, `ParrotSoft` has been annotated with `@Package`, but `Complex` has **not**. This means that the statement `using Complex` will **not** yield the expected result. Since `Complex` is a part of `ParrotSoft`, it can only be accessed from it. Finally, since we haven't imported `StringUtils` explicitly, we type `ParrotSoft.StringUtils` to access it.
