//
// package.ss
// Package example
// Copyright 2018-2019 Alexandre Martins <alemartf(at)gmail(dot)com>
//

// import the package
using StringUtils;

// An application that uses the imported package
object "Application"
{
    str = "alucard";

    state "main"
    {
        Console.print("Reverse a string:");
        Console.print(str);
        Console.print(StringUtils.reverse(str));
        Application.exit();
    }
}

// Packages are annotated with "@Package"
@Package
object "StringUtils"
{
    fun constructor()
    {
        Console.print("Spawned StringUtils.");
    }

    fun reverse(str)
    {
        buf = "";
        for(i = str.length - 1; i >= 0; i--)
            buf += str[i];
        return buf;
    }
}