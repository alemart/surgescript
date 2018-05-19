//
// custom_plugin.ss
// Custom plugin example (how to extend the language)
// Copyright 2018 Alexandre Martins <alemartf(at)gmail(dot)com>
//

// Plugins are used to extend the language. You can
// import them into your source file using the
// "using" directive:
using StringUtils;

// An application that uses the imported plugin
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

// Plugins are objects annotated with "@Plugin"
// that are spawned automatically. They can be
// imported using the "using" keyword.
@Plugin
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