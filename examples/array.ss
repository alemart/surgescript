//
// hello.ss
// Hello World in SurgeScript
// Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    state "main"
    {
        arr = [1,2,3,"ale",5];
        Console.print(arr.length());
        arr[1]=20;
        arr[0] = 100+1;
        arr[2] = [1,1,1,1].length();
        arr.sort();
        Console.print(arr[0]);
        Console.print(arr[1]);
        Console.print(arr[2]);
        Console.print(arr[3]);
        Console.print(arr[4]);
        Application.exit();
    }
}