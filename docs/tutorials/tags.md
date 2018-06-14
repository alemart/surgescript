Tags
====

You may tag objects with as many tags as you want. Tags are a way of attributing categories to different objects.

Tags should be placed right after the object name. Example:

```
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

We may check if an object has a particular tag with the **hasTag()** function (see the [Object reference](../reference/object#hastag) for more details):

```
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

You may also tag objects with emoticons. The syntax is as follows:

```
object "Good Day" is "happy", "sunny" :)
{
    ...
}

object "Good Night" is <3
{
    ...
}
```

The following emoticons are available for your pleasure:

Emoticon|Meaning
--------|-------
$_$ | Feelin' Rich
<3 | Lovin' it
:) | Happy
:( | Sad
:P | D'oh
