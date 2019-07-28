Functions
=========

Functions are blocks of code that you can reuse throughout your program. They may receive input parameters and they always output something. The syntax is as follows:

```
fun function_name(param_1, param_2, param_3, ..., param_n)
{
    // block of code
    // to be executed
}
```

Functions may be associated to objects. Any function associated to an object may be used (called) by the object itself or by other objects. This means that functions are *public* (they are visible to the outside world).

**Note:** functions should be defined after the states.

Return statement
----------------

Use the `return` statement to stop executing the function and to return some value to the caller. Example:

```
// This function will return the double of input parameter x
fun double(x)
{
    return 2 * x;
    Console.print("This line will never be executed.");
}
```

If no value is specified after the `return` keyword, the function will be stopped and its return value will be `null`. Additionally, if no `return` statement is found, then the return value will also be `null`.

**Note**: an object state may also contain one or more `return` statements to stop the execution of its code. However, return values must not be specified in that context, since states do not return values.

Simple example
--------------

In the example below, we have an object called `Calculator` with two functions: `greet()` and `sum()`. `greet()` takes no input parameters and just greets the user. On the other hand, `sum()` takes two input parameters, `a` and `b`, and returns the sum `a + b`:

```
object "Calculator"
{
    fun greet()
    {
        Console.print("Hello! I am a Calculator!");
    }

    fun sum(a, b)
    {
        return a + b;
    }
}
```

In function `sum()`, variables `a` and `b` hold the input data and are only visible inside the function.

Now, let's create a program that uses this `Calculator`. Remember that the functions defined above can be called by any object, including `Application`:


```
object "Application"
{
    calculator = spawn("Calculator");

    state "main"
    {
        // greet the user
        calculator.greet();

        // do some computation
        result = calculator.sum(5, 7);
        Console.print(result);

        // we're done!
        //Application.exit();
    }
}
```

The output of the above program is:

```
Hello! I am a Calculator!
12
```

Notice that the `Application` does not need to know *how* the functions of the `Calculator` are implemented. It just needs to know *what* they do. This means that you may change the implementation of the `Calculator` without changing the `Application`, and the end-result will stay the same.

Recursive functions
-------------------

In SurgeScript, a function is said to be **recursive** when it calls itself within its code. Recursion can be useful in many ways; namely, when you define a problem in terms of itself. In mathematics, the factorial of a non-negative integer number `n`, denoted as `n!`, is such an example:

```
n! = { n * (n-1)!    if n > 1
     { 1             otherwise
```

One interesting example of recursion that arises in computer science is called the *binary search* algorithm. Given a sorted [Array](/reference/array), the binary search finds the position of a target value without having to inspect the whole collection. In the example below, `bsearch()` is a recursive function:

```
object "BinarySearch"
{
    // Given an array *sorted in ascending order*, find()
    // finds the position of the target value in the array.
    // It returns its index, or -1 if the target is not found.
    fun find(array, value)
    {
        return bsearch(array, value, 0, array.length - 1);
    }

    // Performs a binary search in array[start..end]
    fun bsearch(array, value, start, end)
    {
        // get the middle point
        mid = start + Math.floor((end - start) / 2);

        // compare the middle point to the target value
        if(start > end)
            return -1; // target not found
        else if(value == array[mid])
            return mid; // found the target value
        else if(value < array[mid])
            return bsearch(array, value, start, mid - 1);
        else
            return bsearch(array, value, mid + 1, end);
    }
}
```

Recursive functions must have at least one base case. A base case is a scenario that does not need recursion to solve the problem. In the factorial example, the base case is `n <= 1` (the factorial is 1). In the binary search example, the base case is `start > end`, meaning that the array is empty and the target value cannot be found, or `value == array[mid]`, meaning that the target value has been found at `mid`.

...as the sages say: *to understand recursion, you must first understand recursion*.

Constructors and destructors
----------------------------

In SurgeScript, constructors and destructors are special functions that are called whenever objects are created and destroyed, respectively. It's not mandatory to define them, but it may be useful to do so (you may want to set up things in your constructors, for example). Neither may receive any arguments (to set up objects with parameters, take a look at [chaining](/tutorials/advanced_features#chaining)).

Let's take our `Calculator` again. In the following example, `constructor()` is called automatically whenever a `Calculator` object is created. Likewise, `destructor()` is called automatically when the object is destroyed.

```
object "Calculator"
{
    fun constructor()
    {
        Console.print("Creating a Calculator...");
    }

    fun destructor()
    {
        Console.print("Destroying a Calculator...");
    }
    
    fun greet()
    {
        Console.print("Hello! I am a Calculator!");
    }

    fun sum(a, b)
    {
        return a + b;
    }
}
```

Now, if we run the `Application` of the previous section without any modification, the output of the program will be:

```
Creating a Calculator...
Hello! I am a Calculator!
12
Destroying a Calculator...
```

Note that, for any object, its `constructor()` runs *before* its main state.
