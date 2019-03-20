Arguments
=========

If you've launched your application via the command-line, this Array-like object can be used to read command-line arguments. This is available at `Application.args`.

The example below prints all the arguments to the screen:

```
// Method 1 (foreach)
foreach(argument in Application.args)
    Console.print(argument);

// Method 2 (for loops)
for(i = 0; i < Application.args.length; i++)
    Console.print(Application.args[i]);

// Or, alternatively:
Console.print(Application.args);
```

Properties
----------

#### length

`length`: number, read-only.

The number of command-line arguments, including the executable.

Functions
---------

#### get

`get(index)`

Gets the specified command-line argument. Instead of calling `get()`, one may use equivalently the `[ ]` operator.

*Arguments*

* `index`: integer number between 0 and `Application.args.length - 1`, inclusive.

*Returns*

A string with the specified command-line argument, or `null` if there is no such argument.

*Example*

```
// Suppose that you run surgescript via the command-line:
// surgescript test_args.ss
executable = Application.args[0]; // "surgescript"
script_file = Application.args[1]; // "test_args.ss"
```

#### option

`option(optionName)`

Gets the value of a certain command-line option.

*Arguments*

* `optionName`: string. The option you want to read.

*Returns*

A string featuring the value of the desired command-line option, or `null` if such an option hasn't been provided by the user.

*Example*

```
// Suppose that you run surgescript via the command-line:
// surgescript test_args.ss --my-option 12345
my_option = Application.args.option("--my-option");

// Options -p and --port are equivalent
// surgescript test_args.ss -p 80
// surgescript test_args.ss --port 80
port = Application.args.option("--port") || Application.args.option("-p");

// Default values (useful if the option is not present)
// surgescript test_args.ss
// surgescript test_args.ss --name alice
name = Application.args.option("--name") || "anonymous";
```

#### hasOption

`hasOption(optionName)`

Checks if the specified option is present in the command-line.

*Arguments*

* `optionName`: string.

*Returns*

Returns `true` if the specified option is present in the command-line; or `false` otherwise.

#### iterator

`iterator()`

Spawns an iterator.

*Returns*

An iterator to loop through the command-line arguments.

#### toString

`toString()`

Converts the command-line arguments to a string.

*Returns*

A string featuring the command-line arguments.