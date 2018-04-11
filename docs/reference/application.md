Application
===========

Represents the Application. You do not need to instantiate it, as it is instantiated automatically. Object `Application` can be accessed by any other object, in any script.

Properties
----------

* `session`: [Session](session) object. Session variables that persist throughout the entire application.
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