Tags
====

You may tag objects with as many tags as you want. Tags are a way of attributing categories to objects.

Syntax
------

Tags should be placed right after the object name. Example:

```cs
object "Horse" is "animal", "vehicle"
{
    state "main"
    {
    }
}

object "Cat" is "animal"
{
    state "main"
    {
    }
}

object "Car" is "vehicle"
{
    state "main"
    {
    }
}
```

Using tags
----------

You may check if an object has a particular tag with the `hasTag()` function (see the [Object reference](/reference/object#hastag) for more details):

```cs
object "Application"
{
    horse = spawn("Horse");
    cat = spawn("Cat");
    car = spawn("Car");

    state "main"
    {
        // Horse is both an animal and a vehicle
        Console.print(horse.hasTag("animal")); // true
        Console.print(horse.hasTag("vehicle")); // true

        // Cat is an animal, but not a vehicle
        Console.print(cat.hasTag("animal")); // true
        Console.print(cat.hasTag("vehicle")); // false

        // Car is a vehicle, but not an animal
        Console.print(car.hasTag("animal")); // false
        Console.print(car.hasTag("vehicle")); // true
    }
}
```