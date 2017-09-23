//
// timeout.ss
// Timeout in SurgeScript
// Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    state "main"
    {
        Console.print("Now on the main state. Please wait 4 seconds...");
        state = "wait1";
    }

    state "wait1"
    {
        if(timeout(4.0)) // will wait 4 seconds to change the state
            state = "cool";
    }

    state "cool"
    {
        Console.print("Cool! Now wait for more 8 seconds...");
        state = "wait2";
    }

    state "wait2"
    {
        if(timeout(8.0))
            state = "done";
    }

    state "done"
    {
        Console.print("You're done!");
        Application.exit();
    }
}