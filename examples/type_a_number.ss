//
// type_a_number.ss
// Type a number plaything
// Copyright 2017 Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    number = 0; // we declare the variable "number" here, so it
                // will be accessible throughout the whole object

    // the Application will be locked in state "main", and will 
    // only leave it if the user types a number between 1 and 9
    state "main"
    {
        Console.write("Type a number between 1 and 9: ");
        number = Number(Console.readline());
        if(number >= 1 && number <= 9)
            state = "done";
    }

    // display the typed number and exit the app
    state "done"
    {
        Console.print("Congratulations! You typed " + number);
        Application.exit();
    }
}