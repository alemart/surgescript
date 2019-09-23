//
// factory.ss
// Factory example
// Copyright 2018 Alexandre Martins <alemartf(at)gmail(dot)com>
//
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
	// a Greeting object for you. Being a package,
	// Greeter can be imported and used anywhere.
	fun call(name)
	{
		g = spawn("Greeting");
		g.name = name;
		return g;
	}
}