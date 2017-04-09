// SurgeScript - Unit Testing
// A Unit Testing Script
// by Alexandre

object "Application"
{
    test = spawn("SurgeScriptTest");

    state "main"
    {
        test.string();
        test.number();
        test.boolean();
        test.objects();
        destroy();
    }
}

object "SurgeScriptTest"
{
    failed = 0;
    tested = 0;
    totalFailed = 0;
    totalTested = 0;

    state "main"
    {
    }




    fun string()
    {
        begin("String");
        test("alexandre".length() == 9) || fail(1);
        test("alexandre".substr(0, 3) == "ale") || fail(2);
        test("alexandre".indexOf("ale") == 0) || fail(3);
        test("alexandre".indexOf("e") == 2) || fail(4);
        test("alexandre".indexOf("z") < 0) || fail(5);
        test("ale" + "xandre" == "ale".concat("xandre")) || fail(6);
        test("alê".length() == 3) || fail(7);
        test("37".toNumber() == 37) || fail(8);
        test("çáàê€".length() == 5) || fail(9);
        test(!"") || fail(10);
        test(("\n" + "\n").length() == 2) || fail(11);
        test((y = "ale", y += "xandre").length() == 9) || fail(12);
        //test("alê\n"[2] == "ê") || fail(13);
        //test("abc"[0] = "b") || fail(14);
        test((y = "abc", y[0] = "b", y == "abc")) || fail(15);
        test((y = "alê", y.substr(0, 1) == "a" && y.substr(2, 1) == "ê")) || fail(16);
        test("hello!") || fail(17);
        test(typeof "string" == "string") || fail(18);
        test(2 + "3" == "23") || fail(19);
        test("2" + 3 == "23") || fail(20);
        end();
    }

    fun number()
    {
        begin("Number");
        test(1 == 1) || fail(1);
        test(2 + 2 == 4) || fail(2);
        test((3 * 3).toString() == "9") || fail(3);
        test(4.toString().toNumber() == 4.valueOf()) || fail(4);
        test(5 * "25".toNumber() == 625 / 5) || fail(5);
        test(0 == -0) || fail(6);
        test(-(-(-1)) * -1 == +1) || fail(7);
        test(.5 == 0.5) || fail(8);
        test(.5 == 1/2) || fail(9);
        test(-.5 == 1/-2) || fail(10);
        test(1 * 2 + 3 == 5) || fail(11);
        test(1+1*2 != (1+1)*2) || fail(12);
        test(0 < 1) || fail(13);
        test(0 <= 0) || fail(14);
        test(0 >= 0) || fail(15);
        test(1 > 0) || fail(16);
        test(5*7 >= 7*5) || fail(17);
        test(5*7 <= 7*5) || fail(18);
        test((8 - 7) * (8 - 7) == 8*8 - 2*8*7 + 7*7) || fail(19);
        test((5 + 25) * (5 + 25) == 5*5 + 2*5*25 + 25*25) || fail(20);
        test(typeof 5 == "number") || fail(21);
        test(typeof(5 + 5) == "number") || fail(22);
        end();
    }

    fun boolean()
    {
        begin("Boolean");
        test(true) || fail(1);
        test(!false) || fail(2);
        test(false == null) || fail(3);
        test((1 == 1).toString() == "true") || fail(4);
        test(!!!!!true == !!false) || fail(5);
        test(false == 0) || fail(6);
        test(1 == true) || fail(7);
        test(false != true) || fail(8);
        test(!!"hi" == true) || fail(9);
        test(true + true == 2) || fail(10);
        test(typeof true == "boolean") || fail(11);
        test(typeof true == typeof false) || fail(12);
        end();
    }

    fun objects()
    {
        begin("Object");
        test(typeof this == "object") || fail(1);
        test(this.name() == "SurgeScriptTest") || fail(2);
        test(app != null && app.name() == "Application") || fail(3);
        test(this != app) || fail(4);
        test(this.toString() == "[object]") || fail(5);
        test(this.hasMemberFunction("objects")) || fail(6);
        test(!this.hasMemberFunction("nope")) || fail(7);
        test(findChild("nope") == null) || fail(8);
        test((c = spawn("Boolean"), c != null && child("Boolean") == c)) || fail(9);
        test((d = spawn("Number"), d != null && d == findChild("Number"))) || fail(10);
        end();
    }





    fun __constructor()
    {
        print("------------------------------");
        print("   SurgeScript Unit Testing   ");
        print("------------------------------");
    }

    fun __destructor()
    {
        str = "\n";
        str += "SUCCEEDED  " + (totalTested - totalFailed) + "\t\t";
        str += "FAILED  " + totalFailed;
        print(str);
    }

    fun test(expr)
    {
        tested++;
        return expr;
    }

    fun fail(x)
    {
        print("# Test " + x + " has failed.");
        failed++;
    }

    fun begin(suite)
    {
        print("\nTesting " + suite + "...");
        tested = 0;
        failed = 0;
    }

    fun end()
    {
        if(!failed)
            print("All tests have passed.");
        totalTested += tested;
        totalFailed += failed;
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

