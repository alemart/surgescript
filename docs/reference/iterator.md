Iterator
========

In SurgeScript, Iterator is an abstract contract, or protocol, that define a set of functions that should be written in concrete implementations. If you're familiar with object oriented programming, Iterator is an interface. Iterators are used to traverse containers / collections / data structures and access its elements.

Concrete implementations of Iterator include: `ArrayIterator` and `DictionaryIterator`. If you wish to [write your own iterators](/tutorials/advanced_features#iterators), you need to specify the [functions described below](#functions).

The data structure that you wish to traverse should include an `iterator()` function that spawns the concrete implementation of the Iterator. Thus, the `parent` object of the Iterator will be the structure to be traversed.

The basic usage of an Iterator is as follows:

```
// container is an object such as an Array
iterator = container.iterator(); // spawn an iterator to traverse the container
while(iterator.hasNext()) {
    element = iterator.next();
    Console.print(element);
}
```

Functions
---------

#### hasNext

`hasNext()`

Checks if there are more elements to be accessed in the iteration.

*Returns*

Returns `true` if there are more elements to be visited, or `false` otherwise.

#### next

`next()`

Gets the next element of the container and advances the iteration.

*Returns*

The next element of the container, or `null` if there is no such element. The first call to `next()` returns the first element of the container, and so on.