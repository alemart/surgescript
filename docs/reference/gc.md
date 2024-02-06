GC
==

SurgeScript features a Garbage Collector (GC) that automatically disposes objects that cannot be reached from the root (i.e., their references are lost). The Garbage Collector is available at `System.gc`. Generally, you do not need to modify any of its settings.

Properties
----------

#### interval

`interval`: number, read-only.

Every `interval` seconds, the garbage collector will be called automatically.

*Note:* this property is read-only since SurgeScript 0.6.0.

#### objectCount

`objectCount`: number, read-only.

How many objects were disposed when the garbage collector was last called.

Functions
---------

#### collect

`collect()`

Calls the Garbage Collector manually. You generally don't need to call this.
