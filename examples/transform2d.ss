//
// transform2d.ss
// This script demonstrates how the Transform2D works with attached objects
// Copyright 2017 Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    // this demo shows 2 bees: a parent and its child.
    // The child bee is always attached to its parent.
    // This means that, wherever the parent moves, the child follows.
    // However, the child may move by itself without affecting its parent.
    parent = spawn("Bee");
    child = parent.spawn("Bee");

    state "main"
    {
        // introduction
        say("# Transform2D - Attached Objects");
        say("# Wherever the parent goes, the child follows.\n");

        // demo
        say("step 1: both bees start at the origin (0,0)");
        showBees();

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
        parent.transform.translate(-2, -1);
        showBees();

        say("step 6: rotate the parent bee by 90 degrees");
        parent.transform.rotate(90);
        showBees();

        say("step 7: scale the transform of the parent bee by 10");
        parent.transform.scale(10, 10);
        showBees();

        say("step 8: move the child bee by (2,0)");
        child.transform.translate(2,0);
        showBees();

        say("step 9: scale the transform of the parent bee by 0.1 (thus reverting the previous scale)");
        parent.transform.scale(0.1, 0.1);
        showBees();

        say("step 10: move the child bee by (-2,0)");
        child.transform.translate(-2,0);
        showBees();

        say("step 11: reset the rotation of the parent bee");
        parent.transform.angle = 0;
        showBees();

        say("step 12: set the child bee to be at position (50,50) in the world");
        child.transform.worldX = 50;
        child.transform.worldY = 50;
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

    // get_transform()
    // allows the transform to be accessed externally, e.g., bee.transform.xpos = 10;
    fun get_transform()
    {
        return transform;
    }

    // get_location()
    // returns a nice "(x,y)" string displaying where the bee is in the world
    fun get_location()
    {
        return "(" + transform.worldX + "," + transform.worldY + ")";
    }
}