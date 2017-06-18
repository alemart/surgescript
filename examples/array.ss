//
// array.ss
// Arrays in SurgeScript
// Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    arr = [1, 1, 2, 3, 5, 8, 13, 21, 34];
    
    state "main"
    {
        i = 0;
        while(i < arr.length()) {
            Console.print(arr[i]);
            i++;
        }
        Application.exit();
    }
}