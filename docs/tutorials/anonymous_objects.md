Anonymous objects
=================

You can encapsulate data into a single immutable object by means of a convenient syntax featuring named fields. When defining such an object, the SurgeScript compiler will generate a class of objects containing the specified fields. A new object of the generated class will be spawned and initialized with the provided values. Such object is called an Anonymous object.

*Available since:* SurgeScript 7

Syntax
------

Anonymous objects may be created with the following syntax:

```cs
obj = {
    field_1 = value_1,
    field_2 = value_2,

    // ...

    field_n = value_n
};
```

Let's take a look at an example:

```cs
object "Application"
{
    // An Anonymous object holding some data
    hero = {
        name = "Surge",
        energy = 100,
        score = 5000
    };

    // print the data
    state "main"
    {
        Console.print(hero.name); // will print Surge
        Console.print(hero.energy); // will print 100
        Console.print(hero.score); // will print 5000

        Application.exit();
    }
}
```

For the purpose of learning, the above example can be thought of as similar to the following:

```cs
object "My Hero"
{
    public readonly name = "Surge";
    public readonly energy = 100;
    public readonly score = 5000;
}

object "Application"
{
    hero = spawn("My Hero");

    state "main"
    {
        Console.print(hero.name);
        Console.print(hero.energy);
        Console.print(hero.score);

        Application.exit();
    }
}
```

!!! warning "Important"

    Whenever you define an Anonymous object, you spawn a new object. It's recommended to **not** define Anonymous objects within states, because the code within states run continuously. New objects will be created at every frame, not just once. In the above example, the Anonymous object is an [object-level variable](./variables.md#scoping). Hence, it's spawned only once.

Abbreviated syntax
------------------

If the name of a field matches the name of a variable in [scope](./variables.md#scoping), then you can omit the initialization of that field. Example:

```cs
// Suppose we've defined variables "name" and "energy" as follows:
name = "Surge";
energy = 100;

// then, instead of writing this:
hero = { name = name, energy = energy };

// we can just write this:
hero = { name, energy };

// no variable named "score" is defined, but we can still define such field:
hero = { name, energy, score = 5000 };
```

Immutability
------------

Anonymous objects are immutable. All their fields are [public and read-only properties](./properties.md). While you can't change their fields directly, you can spawn new Anonymous objects with new values:

```cs
//hero1.score += 100; // illegal because hero1 is immutable

// assigning hero1 to a new Anonymous object is legal
hero1 = {
    name = hero1.name,
    energy = hero1.energy,
    score = hero1.score + 100,
};
```

Note that you can't add methods or states to Anonymous objects.

Nesting
-------

Anonymous objects can contain other Anonymous objects as fields. Example:

```cs
game = {
    hero = {
        name = "Surge",
        energy = 100,
        score = 5000
    },
    boss = {
        name = "Gimacian",
        attack = 500
    }
};

Console.print("The boss has " + game.boss.attack + " attack points.");
```

Shapes
------

The shape of an Anonymous object is its set of field names, plus the order in which they are specified. The SurgeScript compiler will only generate different classes of objects for Anonymous objects of different shapes.

In the example below, `hero1` and `hero2` have the same shape, even though the values they encapsulate differ. On the other hand, `boss` has a different shape:

```cs
hero1 = {
    name = "Surge",
    energy = 100,
    score = 5000,
};

hero2 = {
    name = "Neon",
    energy = 80,
    score = 2000,
};

boss = {
    name = "Gimacian",
    attack = 500,
};
```

Equality
--------

The SurgeScript compiler defines an [equals](../reference/object.md#equals) method for Anonymous objects. Two Anonymous objects are considered equal if they have the same [shape](#shapes) and if the values they encapsulate are equal.

```cs
p1 = { x = 100, y = 200 }; // reference

p2 = { x = 100, y = 200 }; // same shape and values as p1
p3 = { x = 50 + 50, y = p1.y }; // same shape and values as p1
p4 = { x = 1, y = 2 }; // same shape, but different values

Console.print(p1.equals(p2)); // true
Console.print(p1.equals(p3)); // true
Console.print(p1.equals(p4)); // false

q1 = { y = 200, x = 100 }; // same fields, but different order
q2 = { x = 100, y = 200, z = 300 }; // different fields

Console.print(p1.equals(q1)); // false
Console.print(p1.equals(q2)); // false
```

Comparison to Dictionaries
--------------------------

Anonymous objects are comparable to [Dictionaries](../reference/dictionary.md), in the sense that both can be used to store data by means of named fields. However, they differ in important ways:

|   | Anonymous objects | Dictionaries |
| - | ----------------- | ------------ |
| **Instantiation** | `obj = { field = value };` | `dict = { "field": value };` |
| **Access** | `obj.field` | `dict["field"]` |
| **Mutability** | Read-only | Read-write |
| **[Iterable](./advanced_features.md#iterators)** | No | Yes |
| **Performance** | Fastest | Good |

Prefer using Anonymous objects over Dictionaries whenever possible.