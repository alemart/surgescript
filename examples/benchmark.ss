//
// benchmark.ss
// Benchmarking in SurgeScript
// Copyright (C) 2018  Alexandre Martins <alemartf(at)gmail(dot)com>
//

//
// A functor is an object that behaves like a function.
// SurgeScript has a syntax sugar that allows an object to be "called" like a function.
// Example: given an object "obj", writing
//     x = obj(param);
// is the same as writing
//     x = obj.call(param);
//

// Main Application
object "Application"
{
    benchmark = spawn("Benchmark");
    fib = spawn("Fibonacci");

    state "main"
    {
        t  = benchmark(fib(1));
        t += benchmark(fib(5));
        t += benchmark(fib(10));
        t += benchmark(fib(25));
        t += benchmark(fib(32));
        Console.print("Total time: " + t + " seconds.");
        Application.exit();
    }
}

// This object benchmarks functor f
object "Benchmark"
{
    fun call(f)
    {
        Console.print("Computing " + f + "...");
        start = Time.tickCount;
        result = f();
        elapsed = Time.tickCount - start;
        Console.print(f + " = " + result + " took " + elapsed + " seconds.");
        return elapsed;
    }
}

// This functor computes the Fibonacci sequence with an exponential algorithm
object "ExponentialFibonacci"
{
    public n = 1;

    fun call()
    {
        return fib(n);
    }

    fun fib(n)
    {
        if(n > 2)
            return fib(n-1) + fib(n-2);
        else
            return 1;
    }

    fun toString()
    {
        return "ExpFib(" + n + ")";
    }
}

// This object generates a Fibonacci functor
object "Fibonacci"
{
    fun call(n)
    {
        fib = spawn("ExponentialFibonacci");
        fib.n = n;
        return fib;
    }
}