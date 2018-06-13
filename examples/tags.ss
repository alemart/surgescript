//
// tags.ss
// Tags in SurgeScript
// Copyright 2017-2018 Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    banana = spawn("Banana");
    coin = spawn("Coin");

    state "main"
    {
        Console.print("Welcome!");
        showStatus(banana);
        showStatus(coin);
        Application.exit();
    }

    fun showStatus(obj)
    {
        if(obj.hasTag("fruit"))
            Console.print("Object " + obj.__name + " is a fruit.");
        else
            Console.print("Object " + obj.__name + " is not a fruit.");
    }
}

object "Banana" is "pickup", "fruit"
{
    state "main"
    {
    }
}

object "Coin" is "pickup"
{
    state "main"
    {
    }
}