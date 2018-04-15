//
// component.ss
// Basic Component usage
// Copyright 2018 Alexandre Martins <alemartf(at)gmail(dot)com>
// 

// Object Parrot has two components:
// Blabber and Time Bomb.
object "Parrot"
{
    blabber = spawn("Blabber");
    bomb = spawn("Time Bomb");

    state "main"
    {
    }
}

object "Blabber"
{
    state "main"
    {
        if(timeout(2)) // blab every 2 seconds
            state = "blab";
    }

    state "blab"
    {
        Console.print("Hello!");
        state = "main";
    }
}

object "Time Bomb"
{
    state "main"
    {
        if(timeout(15)) // explode after 15 seconds
            state = "explode";
    }

    state "explode"
    {
        Console.print("BOOOM!");
        parent.destroy(); // destroy the parent object
        Application.exit();
    }
}

object "Application"
{
    parrot = spawn("Parrot");

    state "main"
    {
    }
}