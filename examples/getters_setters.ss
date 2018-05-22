//
// getters_setters.ss
// getters & setters in SurgeScript
// Copyright 2017 Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    obj = spawn("Skull");

    state "main"
    {
        hello();
        showStatus();
        obj.eyes = 4; // the same as obj.set_eyes(4)
        //obj.name = "master"; // will crash; no setter defined. Try uncommenting this.
        showStatus();
        Application.exit();
    }

    fun hello()
    {
        // typing obj.name is the same as obj.get_name()
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

    // get_eyes()
    // syntax sugar: typing obj.eyes yields the same as obj.get_eyes()
    fun get_eyes()
    {
        return eyes;
    }

    // set_eyes()
    // syntax sugar to obj.eyes = value (will call obj.set_eyes(value) behind the scenes)
    fun set_eyes(value)
    {
        eyes = value;
    }

    // get_name()
    // obj.name will return the [private] variable name. Notice we did not define a setter.
    fun get_name()
    {
        return name;
    }
}