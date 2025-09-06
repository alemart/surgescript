Advanced features
=================

This section describes advanced features of SurgeScript.

Lookup operator
---------------

Some programming languages, such as C++, have a feature called *operator overloading*. It's a *syntactic sugar* that allows the programmer to attribute custom implementations to different operators.

In SurgeScript, the `[]` operator (also called the *lookup operator*), used by [Arrays](/reference/array) and [Dictionaries](/reference/dictionary), is used to **get** and **set** values from/to the data structure. In fact, the `[]` operator can be used with any object. It is necessary to define, in your object, functions named `get()` and `set()` with the following signature:

```cs
fun get(key)
{
    // custom implementation
}

fun set(key, value)
{
    // custom implementation
}
```

Given an object `obj`, the expression `x = obj[key]` is equivalent to `x = obj.get(key)`. Similarly, `obj[key] = value` is equivalent to `obj.set(key, value)`.

Function objects
----------------

In SurgeScript, objects can be made to behave like functions. We call these objects *function objects* (or *functors*). To make an object behave like a function, you have to overload the `()` operator (also known as the *function operator*). This is done by defining function `call()` in your object:

```cs
fun call()
{
    // custom implementation
}
```

Function `call()` may take any number of parameters. Given an object `f`, the expression `y = f(x)` is equivalent to `y = f.call(x)`. Notice that, since `f` is an object, you may exchange its implementation during runtime.

Function objects have many uses, including [events](/engine/functionevent) and [sorting arrays](/reference/array#sort).

Assertions
----------

The `assert(condition)` statement specifies a `condition` that you expect to be true at a certain point of your program. If that condition turns out to be false, the program will be interrupted with an *assertion failed* error. Example:

```cs
// will crash if name isn't "Surge"
assert(name == "Surge");
```

Since SurgeScript 0.6.1, you may add a `message` to your assertion, as in `assert(condition, message)`. Your `message` must be a string literal. It will be displayed in a crash if the assertion fails. Example:

```cs
// crash with a message if the assertion fails
assert(name == "Surge", "The name must be Surge");
```

Chaining
--------

In SurgeScript, it's possible to configure objects in an elegant way using a technique called *chaining*. Consider the object below - it simply displays a message at regular intervals:

```cs
object "Parrot"
{
    message = "I am a Parrot";

    state "main"
    {
        if(timeout(1.0))
            state = "print";
    }

    state "print"
    {
        Console.print(message);
        state = "main";
    }

    // Note that this function returns
    // this, i.e., the object itself.
    fun setMessage(newMessage)
    {
        message = newMessage;
        return this;
    }
}
```

Suppose that, in your Application, you would like to spawn that object and modify its message. One way of doing it would be making its internal variable `public` and changing its contents in the [constructor function](/tutorials/functions) of your Application. A more concise and elegant way of doing it would be calling function `setMessage()` just after you spawn the object:

```cs
object "Application"
{
    parrot = spawn("Parrot").setMessage("Hello!");

    state "main"
    {
    }
}
```

Observe that the function we have defined does two things:

* It modifies the internals of the object in some way
* It always returns `this` (that is, the object itself)

We call that function a *chainable function*. You may call such a function from your Application, just after `spawn()`, and you'll still have a reference to the spawned object. Moreover, since chainable functions always return `this`, you may chain multiple function calls into a single statement, making your code concise and your statement descriptive. Example:

```cs
parrot = spawn("Parrot").setMessage("Hello!").setInterval(2.0);
```

Factory
-------

In SurgeScript, a factory is a functor that spawns an object for you. The object can be spawned and configured in a single call. In the example below, factory `Greeter` spawns and configures `Greeting` objects. We annotate the factory with `@Package`, so it can be imported anywhere in the code.

To the end-user, calling `Greeter()` is simpler than manually spawning and configuring a `Greeting` every time it is needed.

```cs
// Factory example
using Greeter; // import the factory

object "Application"
{
    state "main"
    {
		// This will print:
		// Hello, alex!
		g = Greeter("alex");
		g.greet();
        exit();
    }
}

// File: greeting.ss
object "Greeting"
{
	public name = "anon";

	fun greet()
	{
		Console.print("Hello, " + name + "!");
	}
}

@Package
object "Greeter"
{
	// Greeter is a factory. It spawns and configures
	// a Greeting object for you. As it is a package,
	// it can be imported and used anywhere.
	fun call(name)
	{
		g = spawn("Greeting");
		g.name = name;
		return g;
	}
}
```

In the example above, objects spawned by the factory will be children of the factory. If you need the parent of the spawned object to be the caller, simply write `g = caller.spawn("Greeter")`. Know that `caller` points to the object that called the function (or `null` if not applicable).

Iterators
---------

As seen in the [loops](/tutorials/loops#foreach) section, the foreach loop may be used to iterate through an iterable collection. In SurgeScript, an iterable collection is an object that implements the iterator protocol described below.

You may implement your own iterable collections by tagging them as `"iterable"` and implementing function `iterator()`. If you have ever used Java, you'll find this to be familiar.

```cs
// Iterable collections are tagged "iterable"
// and implement function iterator()
object "MyCollection" is "iterable"
{
    fun iterator()
    {
        // function iterator() takes no arguments and 
        // returns a new iterator object
    }
}
```

For each iterable collection you define, you must define its iterator object. The iterator object must be tagged `"iterator"` and implement functions `next()` and `hasNext()` (both take no arguments):

```cs
// Iterators are tagged "iterator" and
// implement functions next() and hasNext()
object "MyIterator" is "iterator"
{
    fun next()
    {
        // returns the next element of the collection
        // and advances the iteration pointer
        // the iterable collection is usually the parent
        // object, i.e., collection = parent
    }

    fun hasNext()
    {
        // returns true if the enumeration isn't over
        // returns false if there are no more elements
    }
}
```

You may iterate over an iterable collection using the following code:

```cs
it = collection.iterator();
while(it.hasNext()) {
    x = it.next();
   
    // do something with x
    // x is an element of the collection
    Console.print(x);
}
```

Or, alternatively, using the compact foreach:

```cs
foreach(x in collection) {
    Console.print(x);
}
```

For the sake of completion, the following code demonstrates how to implement a custom iterable collection that hold even numbers from 0 up to 20 without having to store them explicitly in memory:

```cs
object "Application"
{
    evenNumbers = spawn("Even Numbers").upTo(20);

    state "main"
    {
        // print all the numbers of the iterable collection
        foreach(number in evenNumbers)
            Console.print(number);

        // we're done!
        exit();
    }
}

object "Even Numbers" is "iterable"
{
    lastNumber = 0;

    fun iterator()
    {
        return spawn("Even Numbers Iterator").upTo(lastNumber);
    }

    fun upTo(num)
    {
        // upTo() is a chainable function that
        // is NOT part of the iterator protocol
        // (but it's useful for this example)
        lastNumber = Number(num);
        return this;
    }
}

object "Even Numbers Iterator" is "iterator"
{
    nextNumber = 0;
    lastNumber = 0;

    fun next()
    {
        currentNumber = nextNumber;
        nextNumber += 2;
        return currentNumber;
    }

    fun hasNext()
    {
        return nextNumber <= lastNumber;
    }

    fun upTo(num)
    {
        // upTo() is a chainable function that
        // is NOT part of the iterator protocol
        // (but it's useful for this example)
        lastNumber = Number(num);
        return this;
    }
}
```

The output of this code is:

```
0
2
4
6
8
10
12
14
16
18
20
```

!!! tip "Challenge!"

    Can you write an iterable collection called *Fibonacci Sequence* containing the first *N* [Fibonacci numbers](https://en.wikipedia.org/wiki/Fibonacci_number) without storing them all explicitly in memory? It should be used as follows:

    ```cs
    // Desired output (for N=10): 0 1 1 2 3 5 8 13 21 34
    sequence = spawn("Fibonacci Sequence").ofLength(10);
    foreach(number in sequence)
        Console.print(number);
    ```
