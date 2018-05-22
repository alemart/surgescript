Application
===========

Represents the Application. You do not need to instantiate it, as it is instantiated automatically. This object can be accessed by any other object, in any script, simply by typing `Application`.

Properties
----------

* `args`: [Arguments](arguments) object. Use it to read command-line arguments.

Functions
---------

#### exit

`exit()`

Exits the Application.

#### crash

`crash(message)`

Exits the Application with an error message.

*Arguments*

* `message`: string. The message to be displayed to the user.

#### destroy

`destroy()`

A synonym for `Application.exit()`.