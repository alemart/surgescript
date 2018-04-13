object "Application"
{
    evenNumbers = spawn("EvenNumbers");
    p = spawn("Parent");

    state "main"
    {
        test();
        state = "2";
    }

    state "2" {

        // iterate through the collection
        foreach(number in evenNumbers)
            Console.print(number);

        // exit the app
        Application.exit();
    }



    fun test()
    {
        Console.print(Application.args);
        Console.print(Application.args.__data);
        Console.print(Application.args.option('--level') || 88);
        Console.print("ae> \"" + "SurgeScript".substr(4, 2) + "\"");
        Console.print("1" == 1);
        Console.print("1" === 1);
        Console.print("----------aa");
    }


}

object "Parent"
{
    child = spawn("Child");
    otherChild = spawn("Child");

    state "main"
    {
        Console.print("Object Parent has " + childCount + " children.");
        child.destroy();
        state = "2";
    }

    state "2"
    {
        Console.print("Now, childCount is " + childCount + ".");
        destroy();
    }
}

object "Child"
{
    state "main"
    {
    }
}

object "EvenNumbers"
{
    fun iterator()
    {
        return spawn("EvenIterator");
    }
}

object "EvenIterator"
{
    nextNumber = 0;

    fun next()
    {
        currentNumber = nextNumber;
        nextNumber += 2;
        return currentNumber;
    }

    fun hasNext()
    {
        return nextNumber <= 20;
    }
}