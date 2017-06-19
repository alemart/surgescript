//
// array.ss
// Arrays in SurgeScript
// Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
//

// The following program will print all the elements of the array
object "Application"
{
    arr = [0, 1, 1, 2, 3, 5, 8, 13, 21, 34];
    
    state "main"
    {
        Console.print("The array has " + arr.length() + " elements.");

        // i is an index ranging from 0 to 9
        for(i in arr) {
            Console.print(arr[i]);
        }

        Application.exit();
    }
}