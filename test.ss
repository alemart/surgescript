// test
// by alemart

object "Application" {
	x = 0;
	math = spawn("Math").init("first");
	//arr = spawn("Array"); // todo: test me

	state "main" {
		x = math;
		print(math[5] += 2);
		destroy();
	}

	state "main2" {
		print("x = " + x + ", math = " + math);
		print(math ? "math vale algo: " + typeof(math) : "math vale nulo");
		if(++x >= 30) {
			if(math) {
				print("pi = " + math.pi());
				math = math.destroy();
			}
		}

        if(x >= 100)
			exit();
	}

	fun __constructor() {
		spawn("Math").init("segundo math");
	}

	fun __destructor() {
		print("no more app!");
		if(1 == 1) if(2 == 2) print("ambos"); else print("soh o primeiro"); else print("nenhum");
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

	fun get(x)
	{
		return x*x;
	}

	fun set(key, value)
	{
		app.print('key=' + key + ', value=' + value);
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

