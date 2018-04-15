//
// alfred_the_npc.ss
// Talk to Alfred, the NPC.
// Copyright 2017 Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Alfred"
{
    // Alfred will ask a series of questions and
    // answer according to the answers of the user.
    state "main"
    {
        say("Hey! I'm Alfred, the NPC. Let's talk!");
        state = "question 1";
    }

    // --- Question 1: Pizza ---
    state "question 1"
    {
        x = ask("I enjoy pizza a lot. Do you?");
        if(x == "y")
            state = "question 1 - yes";
        else if(x == "n")
            state = "question 1 - no";
    }

    state "question 1 - yes"
    {
        say("Cool! Who doesn't?");
        state = "question 2";
    }

    state "question 1 - no"
    {
        say("Well, too much pizza is bad for you anyway.");
        state = "question 2";
    }

    // --- Question 2: Coke ---
    state "question 2"
    {
        x = ask("How about coke? Do you like it?");
        if(x == "y")
            state = "question 2 - yes";
        else if(x == "n")
            state = "question 2 - no";
    }

    state "question 2 - yes"
    {
        say("Yikes! Gives me hickups!");
        state = "question 3";
    }

    state "question 2 - no"
    {
        say("Too bad!");
        state = "question 3";
    }

    // --- Question 3: Your name ---
    state "question 3"
    {
        say("What's your name, pal?");
        name = Console.readline();
        if(name != "") {
            say("It's been a pleasure talking to you, " + name);
            Application.exit();
        }
    }


    // say()
    // Say something to the user
    fun say(message)
    {
        Console.print(message);
    }

    // ask()
    // Asks a question on the console and returns "y" (yes), "n" (no) or "" (no answer)
    fun ask(question)
    {
        Console.write(question);
        Console.write(" (y/n) ");
        answer = Console.readline();

        if(answer == "y" || answer == "Y" || answer == "yes" || answer == "YES")
            return "y";
        else if(answer == "n" || answer == "N" || answer == "no" || answer == "NO")
            return "n";
        else
            return "";
    }
}

object "Application"
{
    alfred = spawn("Alfred");

    // do nothing.
    state "main"
    {
    }
}