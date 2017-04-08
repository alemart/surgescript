// SurgeScript - Unit Testing
// A Unit Testing Script
// by Alexandre

object "Application"
{
    test = spawn("SurgeScriptTest");

    state "main"
    {
        test.string();
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
        //test((y = "ale", y += "xandre").length() == 9) || fail(12);
        //test("alê\n"[2] == "ê") || fail(13);
        //test("abc"[0] = "b") || fail(14);
        test((y = "abc", y[0] = "b", y == "abc")) || fail(15);
        test(y.substr(0, 1) == "a") || fail(16);
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
        str = str + "SUCCEEDED  " + (totalTested - totalFailed) + "\t\t";
        str = str + "FAILED  " + totalFailed;
        //str += "FAILED  " + totalFailed;
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

