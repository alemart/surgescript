//
// anonymous_objects.ss
// How to use Anonymous Objects in SurgeScript
// Copyright 2026 Alexandre Martins <alemartf(at)gmail(dot)com>
//

object "Application"
{
    state "main"
    {
        play();
        Application.exit();
    }

    fun play()
    {
        // Declare an Anonymous Object like this
        // This object contains fields related to a hero
        hero = {
            hearts = 3,
            coins = 10,
        };

        // Let's show it
        showHero(hero);

        // Let's create a new hero
        hero2 = createHero(3, 10); // try changing a number!
        Console.print("A new hero was born! " + hero2);

        // We can compare Anonymous Objects like this
        if(hero.equals(hero2))
            Console.print("Both heroes have the same fields, declared in the same order.");
        else
            Console.print("Different heroes!");

        // Anonymous Objects are immutable, unlike Dictionaries (which are mutable)
        // This being said, you can create new Anonymous Objects with new values
        hero = {
            hearts = hero.hearts + 1,
            coins = hero.coins,
            score = 100 // a new object with a new field
        };
        // hero.hearts += 1; // illegal because Anonymous Objects are immutable

        // You can also create nested Anonymous Objects like this
        game = {
            hero,
            boss = {
                attack = 100,
                magic = 50
            }
        };
        Console.print("The boss has " + game.boss.attack + " attack points.");
    }

    fun createHero(hearts, coins)
    {
        // You can also create Anonymous Objects with this shorter syntax!
        // The fields will receive the values of the corresponding variables
        return { hearts, coins };
    }

    fun showHero(hero)
    {
        // Extract values
        hearts = hero.hearts;
        coins = hero.coins;

        // Print a message
        Console.print("This hero has " + hearts + " hearts and " + coins + " coins.");

        // Convert to string and print
        str = hero.toString();
        Console.print(str);
    }
}
