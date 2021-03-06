//
// arguments.ss
// Print the command line arguments
// Copyright 2018 Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    state "main"
    {
        // this app will print the command line arguments
        Console.print("Command line arguments: " + Application.args.length);

        // using foreach
        Console.print("Using foreach:");
        foreach(argument in Application.args)
            Console.print(argument);

        // using for loops
        Console.print("Using for loops:");
        for(i = 0; i < Application.args.length; i++)
            Console.print(Application.args[i]);

        // done!
        Application.exit();
    }
}
