Console
=======

The Console is a mechanism that allows users to interact with your app via a text-based interface. You can print data to the user and read data from the user.

Functions
---------

#### print

`print(line)`

Prints a line to the console.

*Arguments*

* `line`: string. The message to be printed.

*Example*

```
// this will print "Hello, there!" and quit
object "Application"
{
    state "main"
    {
        Console.print("Hello, there!");
        Application.exit();
    }
}
```

#### write

`write(str)`

Writes a string to the console, without a line break at the end.

*Arguments*

* `str`: string. The string to be written.

#### readline

`readline()`

Reads a line from the standard input.

*Returns*

Returns the string just read, without a line break at the end.

*Example*

```
// will ask the name of the user until a name is given
object "Application"
{
    state "main"
    {
        Console.print("Hey pal, what's your name? ");
        name = Console.readline();
        if(name != "") {
            Console.print("Nice to meet you, " + name);
            Application.exit();
        }
    }
}
```