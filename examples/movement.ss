//
// movement.ss
// Moving an object in SurgeScript
// Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    // a transform allows us to change the position of the object
    transform = spawn("Transform2D");
    
    state "main"
    {
        Console.print("Welcome to the MOVEMENT example!");
        showPosition();
        Console.print("   moving it to the right...");
        state = "moveHorizontally";
    }

    state "moveHorizontally"
    {
        // move the object at a rate of 10 units per second
        transform.xpos += 10.0 * Time.delta;

        // ...during 2.5 seconds
        if(timeout(2.5))
            state = "checkpoint";
    }

    state "checkpoint"
    {
        showPosition();
        Console.print("   moving it along the y-axis...");
        state = "moveVertically";
    }

    state "moveVertically"
    {
        transform.ypos += 10.0 * Time.delta;
        if(timeout(1.5))
            state = "done";
    }

    state "done"
    {
        showPosition();
        Console.print("We're done!!!");
        Application.exit();
    }

    // shows the position of the object
    fun showPosition()
    {
        x = transform.xpos;
        y = transform.ypos;
        Console.print("The object is at (" + x + "," + y + ")");
    }
}