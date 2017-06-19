//
// array.ss
// Arrays in SurgeScript
// Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    arr = [0, 1, 1, 2, 3, 5, 8, 13, 21, 34];
    
    state "main"
    {
        Console.print("The array has " + arr.length() + " elements.");

        for(i in arr) {
            Console.print(arr[i]);
        }

        Application.exit();
    }
}