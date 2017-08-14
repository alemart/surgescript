//
// getters_setters.ss
// getters & setters in SurgeScript
// Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    obj = spawn("Skull");

    state "main"
    {
        hello();
        showStatus();
        //obj.eyes = 4; // the same as obj.setEyes(4)
        //obj.name = "master"; // will crash; no setter defined.
        showStatus();
        Application.exit();
    }

    fun hello()
    {
        // typing obj.name is the same as obj.getName()
        Console.print("Hello, " + obj.name);
    }

    fun showStatus()
    {
        Console.print("Skull " + obj.name + " has " + obj.eyes + " eyes.");
    }
}

object "Skull"
{
    name = "kid";
    eyes = 2;

    state "main"
    {
    }

    fun getEyes()
    {
        return eyes;
    }

    fun setEyes(value)
    {
        eyes = value;
    }

    fun getName()
    {
        return name;
    }
}