// test
// by alemart

object "Application" {
	x = 7;
	sep = "teste 1, 2, 3 !!";
	//n = (1 + 2) * 3;
	n = 0;
	x = ++n;
	n = n;
	t = this;
	t = typeof (t == "object");
	t = typeof (t);
	//x = print("oi");
	//sep = surge(1,2);

	state "main" {
		/*
		b=5; c = 77;
		x=7;
		print("hoho!" + b + "," + b + ">" + (2*b+1));
		print("b=" + b + ", c=" + c);
		print(d = 8); print(d > 0);
		print("x=" + x);
		*/
		//x = x + 1;
		//print(x);
		print("oi");
		print(surge(2,3));
		destroy();
	}

	fun surge(a, b) {
		//print("a=" + (u = a));
		print("u = " + (u = 1 + a*a + b*b));
		//print("b="+(v=b*b));
		return 1 + u;
		return a*a + b*b;
	}
/*
	sep = "\n\n\n";
	x = 3.14 && null || 2 && -0.1;
	y = 2 < 3 || 2;
	z = 1-1+2 && -5 / -2 + 1 - 1 * 2 / -2;
	w = "oi," + " mundo!" * 5;

	flag = true;
	x = "surge" +1;
	x = flag + (1+flag) * 2 + " heeey";
  	number = 0 ? 1 : 3.31459;
	  */
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
