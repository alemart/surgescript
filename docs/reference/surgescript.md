SurgeScript
===========

Data related to the scripting language itself. You can access this object simply by typing `SurgeScript`.

Example:

```
// prints the version of the SurgeScript runtime
object "Application"
{
	state "main"
	{
        Console.print(SurgeScript.version);
		Application.exit();
	}
}
```

Properties
----------

* `version`: string. Version number.