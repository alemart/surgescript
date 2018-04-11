//
// hello.ss
// Hello World in SurgeScript
// Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
//

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
        Console.print(typeof(this+2) + "," + (this+2));
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
    }
}

object "Parrot"
{
    blabber = spawn("Blabber");
    bomb = spawn("Time Bomb");

    state "main"
    {
    }

    fun destructor()
    {
        Application.exit();
    }
}

object "Application"
{
    tag "asset";
    tag "how do i";

    parrot = spawn("Parrot");

    state "main"
    {
        state = "freeze"; return;
        Console.print(typeof true);
        exit();

        Console.print(System.tags.list());
        Console.print(System.tags.select("enemy"));
        Console.print(System.tags.select("*"));
        state = "done";
    }

    state "done"
    {
        //Console.print(dict.keys());
        Console.print("mem: " + __memspent);
        Application.exit();
    }

    state "freeze"
    {
    }

    fun toString()
    {
        return "app";
    }
}

object "Foo" <3
{
    tag "something";
    tag "enemy";

    state "main"
    {
        arr = [1,2,4];
        Console.print("cc:"+System.temp.childCount);
        return;

        Console.print(this.__name);
        Console.print(System.tags);
        //while(!timeout(0.5));
        return;

        dict = ({
            'Surge': 1,
            'Neon': 2,
            'Charge': 3,
            'Gimacian': null //[1, 2, 3, {'Surge': 2}]
        });

        foreach(entry in dict) {
            Console.print(entry.key + " => " + entry.value);
            //if(entry.key == 'Neon') break;
        }

        Console.print(this.hasTag("<3"));
        Console.print(this.hasTag(":)"));
        Console.print(this.hasTag(":("));

    }
}

object "xaaa" :)
{
    tag "enemy";
}