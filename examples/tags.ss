//
// hello.ss
// Hello World in SurgeScript
// Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    obj = spawn("Banana"); // change to "Alfred" or "Coin"

    state "main"
    {
        // check if the object is tagged "Pickup"
        if(obj.hasTag("Pickup"))
            Console.print("The object is a pickup.");
        else
            Console.print("The object is not a pickup.");

        // done!
        Application.exit();
    }
}

object "Alfred"
{
    tag "NPC";

    state "main"
    {
    }
}

object "Banana"
{
    tag "Pickup";
    tag "Fruit";

    state "main"
    {
    }
}

object "Coin"
{
    tag "Pickup";

    state "main"
    {
    }
}