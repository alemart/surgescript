//
// garbage_collector.ss
// Garbage Collector test
// Copyright 2018 Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    count = 0;
    arr = null;

    state "main"
    {
        // this will allocate a lot of phony arrays
        // the Garbage Collector should release them
        phonyAlloc();

        // print the garbage stats every once in a while
        if(timeout(0.5)) {
            Console.print("\nObjects before release: " + System.objectCount);
            System.gc.collect();
            Console.print("Released objects: " + System.gc.objectCount);
            state = "wait";
        }
    }

    state "wait"
    {
        if(timeout(0.5))
            state = "print";
    }

    state "print"
    {
        Console.print("Objects after release: " + System.objectCount);

        if(++count >= 20)
            Application.exit();

        state = "main";
    }

    fun constructor()
    {
        // intro text
        Console.print("Garbage Collector Test");

        // disable automatic garbage collection
        System.gc.interval = Math.infinity;
    }

    fun phonyAlloc()
    {
        if(count < 15)
            arr = [];
    }
}