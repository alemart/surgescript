// test
// by alemart

object "Application" {
	x = 1;
	math = spawn("Math");
	//arr = spawn("Array"); // todo: test me

	state "main" {
		//"hello".hi();
		print(2.toString());
		//destroy();
	}

	fun __destructor() {
		print("no more app!");
	}
}

// ocorre aqui um overlap de enderecos heap no app.
// com os enderecos definidos no compilador.
// alternativa: reservar a 1a celula de cada heap (deixa-la null por padrao*)
// app --> system --> string, number, boolean and so on
// ou so alocar + espaco no app
// * ou aponta-la para o app (heap[0] eh um temp --- __data() to retrieve it? __tmpval())
// ou fazer o root ser system
// ou fazer o garbage collector nao limpar filhos de app (solucao ruim) tageados na engine***

object "Math"
{
	msg = "Simple thing!";

	state "main"
	{
		print(msg);
		//exit();
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

