// test
// by alemart

object "test" {
	"requires": "0.2.0",
	"author": "Alexandre"
	
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

object "MyObject" {
	//. :-) :-P :-o :-( <3 $_$
	//:-P { "author": "Alexandre" }
	//:-o { "author": "Alexandre" }
	//:-) { "I love Surge" : "" }
	//:-* { "I love Surge" : "" }

	//2
	//3.14
	
	//+5.1
	//-5

	/*
	:-) {
		"requires": "0.2.0",
		"author": "Alexandre",
		"editor": "SD_SURGE 0",
		"label": "Hello!"
		"category": "Enemies",
		"hidden": true,
		"awake": true,
	}


	length = 0;
	N = spawn("Notes").get("editor")

	fun test()
	{
		// hey!
		app().exit();
	}
	*/
}
