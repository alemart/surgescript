SurgeScript
===========

Data related to the scripting language itself. You can access this object simply by typing `SurgeScript`.

Example:

```cs
// Prints the version of the SurgeScript runtime
object "Application"
{
	state "main"
	{
        Console.print(SurgeScript.version);
	}
}
```

*Available since:* SurgeScript 0.5.2

Properties
----------

#### version

`version`: string, read-only.

Version number of the SurgeScript runtime.