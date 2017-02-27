// test
// by alemart
// TODO: temps should share one place (for cache opt)

object "test" {
/*
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
*/
}

object "test" {
/*
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
*/
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


/*
 * SurgeScript grammar:
 * (e is the empty symbol)
 *
 *
 *
 * <script> := <objectlist> // start here
 *
 * <objectlist> := <object> <objectlist> | e
 * <object> := object string { <objectdecl> }
 * <objectdecl> := <notelist> <vardecllist> <statedecllist> <fundecllist>
 *
 * <notelist> := <note> <notelist1> | e
 * <notelist1> := , <note> <notelist1> | <endnote>
 * <note> := string : <signedconst>
 * <signedconst> := <signednum> | string | true | false | null
 * <signednum> := + number | - number | number
 * <endnote> := emoticon | e
 *
 * <vardecllist> := <vardecl> <vardecllist> | e
 * <vardecl> := identifier = <expr> ;
 *
 * <statedecllist> := <statedecl> <statedecllist> | e
 * <statedecl> := state string { <stmtlist> }
 *
 * <fundecllist> := <fundecl> <fundecllist> | e
 * <fundecl> := fun identifier ( ) { <stmtlist> }
 *           |  fun identifier ( <funargs> ) { <stmtlist> }
 * <funargs> := identifier <funargs1>
 * <funargs1> := , identifier <funargs1> | e
 *
 *
 *
 * <expr> := <assignexpr> , | <assignexpr>
 * <assignexpr> := <conditionalexpr> | identifier assignop <assignexpr>
 * <conditionalexpr> := <logicalorexpr> | <logicalorexpr> ? <expr> : <conditionalexpr>
 * <logicalorexpr> := <logicalandexpr> <logicalorexpr1>
 * <logicalorexpr1> := || <logicalandexpr> <logicalorexpr1> | e
 * <logicalandexpr> := <equalityexpr> <logicalandexpr1>
 * <logicalandexpr1> := && <equalityexpr> <logicalandexpr1> | e
 * <equalityexpr> := <relationalexpr> <equalityexpr1>
 * <equalityexpr1> := equalityop <relationalexpr> <equalityexpr1> | e
 * <relationalexpr> := <additiveexpr> <relationalexpr1>
 * <relationalexpr1> := relationalop <additiveexpr> <relationalexpr1> | e
 * <additiveexpr> := <multiplicativeexpr> <additiveexpr1>
 * <additiveexpr1> := additiveop <multiplicativeexpr> <additiveexpr1> | e
 * <multiplicativeexpr> := <unaryexpr> <multiplicativeexpr1>
 * <multiplicativeexpr1> := multiplicativeop <unaryexpr> <multiplicativeexpr1> | e
 * <unaryexpr> := + <unaryexpr> | - <unaryexpr>
 *             |  ++ <unaryexpr> | -- <unaryexpr>
 *             |  ! <unaryexpr>
 *             |  typeof <unaryexpr> | typeof ( <unaryexpr> )
 *             |  <postfixexpr>
 * <postfixexpr> := identifier . <postfixexpr>
 *               |  identifier ( ) <postfixexpr>
 *               |  identifier ( <funargsexprlist> ) <postfixexpr>
 *               |  identifier [ <expr> ] <postfixexpr>
 *               |  <primaryexpr>
 * <primaryexpr> := identifier | <constant> | ( <expr> )
 * <constant> := number | string | true | false | null
 * <funargsexprlist> := <assignexpr> <funargsexprlist1>
 * <funargsexprlist1> := , <assignexpr> <funargsexprlist1> | e
 *
 *
 *
 * <stmtlist> := <stmt> <stmtlist> | e
 * <stmt> := <blockstmt>
 *        |  <exprstmt>
 *        |  <condstmt>
 *        |  <loopstmt>
 *        |  <jumpstmt>
 * <blockstmt> := { <stmtlist> }
 * <exprstmt> := ;
 *            |  <expr> ;
 * <condstmt> := if ( <expr> ) <stmt>
 *            |  if ( <expr> ) <stmt> else <stmt>
 * <loopstmt> := while ( <expr> ) <stmt>
 * <jumpstmt> := return <expr> ;
 *            |  return ;
 *            |  break ;
 *            |  continue ;
 *
 */