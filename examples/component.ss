//
// component.ss
// Basic Component usage
// Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
// 

// TestObject has two components: A and B.
// Component B calls Component A.
// How does B access A? We'll see.

object "ComponentB"
{
    a = requireComponent("ComponentA");

    state "main"
    {
        Console.print("Component B will call Component A...");
        a.call();
        destroy();
    }

    // will spawn the object if it doesn't exist
    fun requireComponent(name)
    {
        return sibling(name) || parent.spawn(name);
    }
}

object "ComponentA"
{
    state "main"
    {
    }

    fun constructor()
    {
        Console.print("Spawned Component A");
    }

    fun call()
    {
        Console.print("Called Component A!");
    }
}

object "TestObject"
{
    a = spawn("ComponentA"); // try commenting this line. It will still work.
    b = spawn("ComponentB");

    state "main"
    {
        ;
    }
}

object "Application"
{
    obj = spawn("TestObject");

    state "main"
    {
        if(timeout(1.0))
            exit();
    }
}