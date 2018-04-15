//
// count_to_10.ss
// Count to 10 SurgeScript
// Copyright 2017 Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    state "main"
    {
        // x will go from 1 to 10
        for(x = 1; x <= 10; x++) {
            Console.print(x);
        }

        // done!
        Application.exit();
    }
}