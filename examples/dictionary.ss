//
// dictionary.ss
// How to use the Dictionary in SurgeScript
// Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    // a Dictionary is a set of (key, value) elements
    weight = spawn("Dictionary");
    
    // main state
    state "main"
    {
        // set up some weights ("weight" as in everyday use, that is)
        weight["Surge"] = 35.0;
        weight["Neon"] = 25.0;
        weight["Charge"] = 37.5;

        // print the weights
        Console.print("Surge weighs " + weight["Surge"] + " kg.");
        Console.print("Neon weighs " + weight["Neon"] + " kg.");
        Console.print("Charge weighs " + weight["Charge"] + " kg.");

        // sum up
        totalWeight = computeTotalWeight();
        Console.print("Together, they set up a total of " + totalWeight + " kg.");

        // done!
        Application.exit();
    }

    // this function will add all weights stored in the Dictionary.
    fun computeTotalWeight()
    {
        sum = 0.0;

        it = weight.iterator();
        while(it.hasNext()) {
            sum += weight[it.item];
            it.next();
        }

        return sum;
    }
}