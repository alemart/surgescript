Advanced Features
=================

This section describes advanced features of SurgeScript.

Lookup operator
---------------

Some programming languages, such as C++, have a feature called *operator overloading*. It's a *syntactic sugar* that allows the programmer to attribute custom implementations to different operators.

In SurgeScript, the *[ ]* operator (also called the *lookup operator*), used by Arrays and Dictionaries, is used to **get** and **set** values from/to the data structure. In fact, the *[ ]* operator can be used by any object, for any purpose. It is necessary to define, in your object, functions *get()* and *set()* with the following signature:

```
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

Functors
--------

In SurgeScript, objects can be made to behave like functions. We call these objects *functors*. To make an object behave like a function, you have to overload the *( )* operator (also known as the *function operator*). This is done by defining function *call()* in your object:

```
fun call()
{
    // custom implementation
}
```

Function *call()* may take any number of parameters. Given an object `f`, the expression `y = f(x)` is equivalent to `y = f.call(x)`. Notice that, since `f` is an object, you may exchange its implementation during runtime.

Iterators
---------

As seen in the [loops](loops#foreach) section, the foreach loop may be used to iterate through a collection. In SurgeScript, a collection is an object (but the opposite is not always true). You may implement your own collections by implementing function *iterator()*. If you have ever used Java, you'll find this to be familiar.

```
object "MyCollection"
{
    fun iterator()
    {
        // function iterator() takes no arguments and 
        // should return a new iterator object
    }
}
```

For each collection you define, you should also define its iterator object. The iterator object should implement functions *next()* and *hasNext()* (both take no arguments):

```
object "MyIterator"
{
    fun next()
    {
        // returns the next element of the collection
        // and advances the iteration pointer
        // the collection is usually the parent object
    }

    function hasNext()
    {
        // returns true if the enumeration isn't over
        // returns false if there are no more elements
    }
}
```

You may iterate through a collection using the following code:

```
it = collection.iterator();
while(it.hasNext()) {
    x = it.next();
   
    // do something with x
    // x is an element of the collection
    Console.print(x);
}
```

Or, alternatively, using the compact foreach:

```
foreach(x in collection) {
    Console.print(x);
}
```

For the sake of completion, the following code demonstrates how to implement a custom collection that hold even numbers from 0 up to 20.

```
object "Application"
{
    evenNumbers = spawn("EvenNumbers");

    state "main"
    {
        // iterate through the collection
        foreach(number in evenNumbers)
            Console.print(number);

        // exit the app
        Application.exit();
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
```

The result of this code is:

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