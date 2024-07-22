//
// switch.ss
// Switch statement example
// Copyright 2024 Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    state "main"
    {
        Console.print("The Full Name Game");
        Console.print("------------------");

        Console.print("I can tell you the full name of some characters.");
        Console.print("Type the name of a character to know its full name or type 'x' to quit the program.");
        Console.print("Known characters: Surge, Neon, Charge, Gimacian, Tux.");

        for(;;) {
            // read input
            Console.write("> ");
            input = Console.readline();

            // quit the program?
            if(input == "x" || input == "X")
                break;

            // repeat the prompt if the input is empty
            if(input == "")
                continue;

            // print the full name and repeat the prompt
            printFullName(input);
        }

        Console.print("Bye!");
        Application.exit();
    }

    fun printFullName(name)
    {
        switch(name.toLowerCase()) {
            case "surge":
                Console.print("Surge the Rabbit");
                break;

            case "neon":
                Console.print("Neon the Squirrel");
                break;

            case "charge":
                Console.print("Charge the Badger");
                break;

            case "gimacian":
                Console.print("Gimacian the Dark");
                break;

            case "tux":
                Console.print("Tux the Penguin");
                break;

            default:
                Console.print("Sorry, I don't recognize this name. Try again!");
                break;
        }
    }
}
