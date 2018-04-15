Tags
====

You may tag objects with as many tags as you want. Tags are a way of attributing categories to different objects.

Tags should be placed before any definitions of variables, states or functions. Example:

```
object "Horse"
{
    tag "animal";
    tag "vehicle";

    state "main"
    {
    }
}

object "Cat"
{
    tag "animal";

    state "main"
    {
    }
}

object "Car"
{
    tag "vehicle";

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

You may also convey emotional states to objects using the following syntax:

```
object "Good Day" :)
{
    state "main"
    {
    }
}
```

The object will then be tagged with the specified emoticon(s). The following emoticons are available for your pleasure:

Emoticon|Meaning
--------|-------
$_$ | Feelin' Rich
<3 | Lovin' it
:) | Happy
:( | Sad
:P | D'oh
