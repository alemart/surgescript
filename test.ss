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
        test.system();
        test.lambda();
        test.getset();
        exit();
    }
}

object "SurgeScriptTest"
{
    failed = 0;
    tested = 0;
    totalFailed = 0;
    totalTested = 0;
    value = null;

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
        test("alê\n"[2] == "ê") || fail(13);
        test("abc"[0] = "b") || fail(14);
        test((y = "abc", y[0] = "b", y == "abc")) || fail(15);
        test((y = "alê", y.substr(0, 1) == "a" && y.substr(2, 1) == "ê")) || fail(16);
        test("hello!") || fail(17);
        test(typeof "string" == "string") || fail(18);
        test(2 + "3" == "23") || fail(19);
        test("2" + 3 == "23") || fail(20);
        test("test".substr(2, 0) == "") || fail(21);
        test("test".substr(-1, 9999) == "test") || fail(22);
        test("test".substr(-1, 1) == "t") || fail(23);
        test("rgba"[-1] == "") || fail(24);
        test("rgba"[4] == "") || fail(25);
        test((typeof "string"[2])[2] == (typeof "string")[2]) || fail(26);
        test(typeof "number"[2] != (typeof "number")[2]) || fail(27);
        test(typeof "number"[2] == typeof("number"[2])) || fail(28);
        test((1 == 2 ? "yes" : "no")[1] == "o") || fail(29);
        test((1 != 2 ? "yes" : "no")[1] != "o") || fail(30);
        test("oi" === "oi") || fail(31);
        test("sim" !== "nao") || fail(32);
        end();
        return this;
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
        test("2" != 3) || fail(23);
        test("2" == 2) || fail(24);
        test("2" !== 2) || fail(25);
        test(2 !== 3) || fail(26);
        test(1 === 1) || fail(27);
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
        test(true && true == true) || fail(13);
        test(true && false == false) || fail(14);
        test(false || true == true) || fail(15);
        test(false || false == false) || fail(16);
        test(false && true || true == true) || fail(17);
        test((false && (true || true)) == false) || fail(18);
        test((!true && !false || !false) == true) || fail(19);
        test((!true && (!false || !false)) == false) || fail(20);
        test((!true && !(false && false)) == false) || fail(21);
        test((!true && !false) == !(true || false)) || fail(22);
        test((!true && !!false) == !(true || !false)) || fail(23);
        test(!true == false) || fail(24);
        test(true == !false) || fail(25);
        test(!(true != false) == (true == false)) || fail(26);
        end();
    }

    fun objects()
    {
        begin("Object");
        test(typeof this == "object") || fail(1);
        test(this.name() == "SurgeScriptTest") || fail(2);
        test(Application != null && Application.name() == "Application") || fail(3);
        test(this != Application) || fail(4);
        test(this.toString() == "[object]") || fail(5);
        test(this.hasFunction("objects")) || fail(6);
        test(!this.hasFunction("nope")) || fail(7);
        test(findChild("nope") == null) || fail(8);
        test((c = spawn("Boolean"), c != null && child("Boolean") == c)) || fail(9);
        test((d = spawn("Number"), d != null && d == findChild("Number"))) || fail(10);
        test(typeof null == "null") || fail(11);
        end();
    }

    fun system()
    {
        begin("System objects");
        test(Application) || fail(1);
        test(Application.name() == "Application") || fail(2);
        test(Application.name() != String.name()) || fail(3);
        test((Application = "foo", Application != "foo")) || fail(4); // read-only
        test("foo" != String) || fail(5);
        test(String.name() == "String") || fail(6);
        test(Number.name() == "Number") || fail(7);
        test(Boolean.name() == "Boolean") || fail(8);
        test(String = null || String) || fail(9);
        test(String.length("test") == 4) || fail(10);
        //test(String("test") == "test") || fail(11);
        //test(Number(4) == 4) || fail(12);
        //test(Number("4.3") == 4.3) || fail(13);
        //test(String(4.5) == "4.5") || fail(14);
        //test(Boolean(true && false) == false) || fail(15);
        test(String.destroy() == null && String.name()) || fail(16);
        end();
    }

    fun lambda()
    {
        begin("Lambdas");
        test(this(5) == 5) || fail(1);
        test(this(this) == this) || fail(2);
        test(this(this)(5) == 5) || fail(3);
        test(this(7) == 7) || fail(4);
        test((I = this, I)(1) == 1) || fail(5);
        test(I("ale") == "ale") || fail(6);
        test(this["this"]("ale") == "ale") || fail(7);
        test(this(this)["this"](7) == 7) || fail(8);
        test(this(this)["this"] == this) || fail(9);
        //test(this(this)["this"](this)["value"] == value) || fail(10);
        //test(this["this"]["value"] == value) || fail(11);
        test(this(this)(7) == this.call(this).call(7)) || fail(12);
        end();
    }

    fun getset()
    {
        begin("Getters and setters");
        test(this["this"] == this) || fail(1);
        test(this[this] != this) || fail(2);
        test(this["value"] == value) || fail(3);
        test((this["value"] = 5, this["value"]) == 5) || fail(4);
        test((this["value"] += 1, this["value"]) == 6) || fail(5);
        //test((this["value"]++, this["value"]) == 7) || fail(6);
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

    fun call(expr)
    {
        return expr;
    }

    fun get(key)
    {
        if(key == "this")
            return this;
        else if(key == "value")
            return value;
        else        
            return null;
    }

    fun set(key, newValue)
    {
        if(key == "value") // aka "public property"
            value = newValue;

        return this;    
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

