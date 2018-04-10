Application
===========

Represents the Application. You do not need to instantiate it, as it is instantiated automatically. Object `Application` can be accessed by any other object, in any script.

Properties
----------

* `session` is a [`Dictionary`](dictionary) of session variables that persist throughout the entire application.

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