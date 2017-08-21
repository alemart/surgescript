//
// attached_object.ss
// This script demonstrates how attached objects work
// Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    // the App will show 2 bees: a parent and its child.
    // The child bee is always attached to its parent.
    // This means that, wherever the parent moves, the child follows.
    // However, the child may move by itself without affecting the parent.
    parent = spawn("Bee");
    child = parent.spawn("Bee");

    state "main"
    {
        // introduction
        say("ATTACHED OBJECTS:")
        say("Wherever the parent goes, the child follows.");
        say("");

        // demo
        say("step 1: both bees start at the origin (0,0)");
        showBees()

        say("step 2: move the parent bee by (2,0) relative to its parent (the Application)");
        parent.transform.xpos += 2;
        showBees();

        say("step 3: move the parent bee by (0,1)");
        parent.transform.ypos += 1;
        showBees();

        say("step 4: set the child bee to be at (5,6) relative to its parent (the parent bee)");
        child.transform.xpos = 5;
        child.transform.ypos = 6;
        showBees();

        say("step 5: move the parent bee by (-2,-1)");
        parent.translate(-2, -1);
        showBees();

        say("step 6: rotate the parent bee by 90 degrees");
        parent.rotate(90);
        showBees();

        say("step 7: scale the transform of the parent bee by 10");
        parent.scale(10, 10);
        showBees();

        say("step 8: move the child bee by (1,0)");
        child.translate(1,0);
        showBees();

        say("step 9: scale the transform of the parent bee by 0.1 (thus reverting the previous scale)");
        parent.scale(0.1, 0.1);
        showBees();

        say("step 10: move the child bee by (1,0)");
        child.translate(1,0);
        showBees();

        // done!
        Application.exit();
    }

    // say()
    // say something
    fun say(something)
    {
        Console.print(something);
    }

    // showBees()
    // show where the bees are
    fun showBees()
    {
        Console.print("The parent bee is at " + parent.location);
        Console.print("The child bee is at " + child.location);
        Console.print("     ");
    }
}

object "Bee"
{
    // the transform will hold the position of this object
    // relative to its parent (plus rotation and scale)
    transform = spawn("Transform2D");

    state "main"
    {
    }

    // getTransform()
    // allows the transform to be accessed externally, e.g., bee.transform.xpos = 10;
    fun getTransform()
    {
        return transform;
    }

    // getLocation()
    // returns a nice "(x,y)" string displaying where the bee is in the world
    fun getLocation()
    {
        world = "(" + transform.worldX + "," + transform.worldY + ")";
    }
}