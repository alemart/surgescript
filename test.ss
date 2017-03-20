// test
// by alemart

object "Application" {
	x = 0;
	math = spawn("Math").init("first");

	state "main" {
		y=2;
		print("main: " + x + "," + y + " > pi=" + math);
		x++;
		math = x >= 30 ? null : math;
		x >= 50 ? exit() : 1;
		//state = "test";
		//app.exit();
		//destroy();
	}

	fun __constructor() {
		spawn("Math").init("segundo math");
	}

	fun __destructor() {
		print("no more app!");
	}
}

object "Math"
{
	msg = "";
	foo = spawn("Foo");

	state "main"
	{
		app.print("foo " + msg);
	}

	fun init(m) { msg = m; return this; }

	fun __constructor()
	{
		app.print("Starting up the math...");
	}

	fun __destructor()
	{
		app.print("Terminating the math. " + msg);
	}

	fun fib(x)
	{
		return x > 2 ? fib(x-1) + fib(x-2) : 1;
	}

	fun pi()
	{
		return 3.1415926535;
	}
}

object "Foo" {
	state "main" {
		;
	}
}



/*
object "test" {
	"requires": "0.2.0",
	"author": "Alexandre",
	"date": "2017-02-25,
	"icon": "SD_SURGE",
	"description": "This is a test object" :-)
	
	child = spawn("child");
	test = 3;
	may = 5.2;

	state "main" {
	}

	fun main() {
	}

	fun app() {
	}
}
*/


object "Application2" {
	x = 2;

	state "main" {
		//u;
		destroy();
	}
}


//object "MyObject" {
	/*
	. :-) :-P :-o :-( <3 $_$

	"requires": "0.2.0",
	"author": "Alexandre",
	"editor": "SD_SURGE 0",
	"label": "Hello!"
	"category": "Enemies",
	"hidden": true,
	"awake": true

	length = 0;
	N = spawn("Notes").get("editor")

	fun test()
	{
		// hey!
		app().exit();
	}
	*/
//}

