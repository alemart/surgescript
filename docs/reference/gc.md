GC
==

SurgeScript features a Garbage Collector (GC) that automatically disposes objects that cannot be reached from the root (i.e., their references are lost). The Garbage Collector is available at `System.gc`. Generally, you do not need to modify any of its settings.

Properties
----------

#### interval

`interval`: number.

Every `interval` seconds, the garbage collector will be called automatically.

#### objectCount

`objectCount`: number, read-only.

How many objects were disposed when the garbage collector was last called.

Functions
---------

#### collect

`collect()`

Calls the Garbage Collector manually.