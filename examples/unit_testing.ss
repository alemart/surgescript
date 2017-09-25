//
// unit_testing.ss
// A Unit Testing Script for SurgeScript
// Copyright (C) 2017  Alexandre Martins <alemartf(at)gmail(dot)com>
//

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
        test.array();
        test.dictionary();
        test.session();
        exit();
    }
}

object "SurgeScriptTest"
{
    export message = "Amazing!";
    value = null;

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
        test("alexandre".length == 9) || fail(1);
        test("alexandre".substr(0, 3) == "ale") || fail(2);
        test("alexandre".indexOf("ale") == 0) || fail(3);
        test("alexandre".indexOf("e") == 2) || fail(4);
        test("alexandre".indexOf("z") < 0) || fail(5);
        test("ale" + "xandre" == "ale".concat("xandre")) || fail(6);
        test("alê".length == 3) || fail(7);
        test("37".toNumber() == 37) || fail(8);
        test("çáàê€".length == 5) || fail(9);
        test(!"") || fail(10);
        test(("\n" + "\n").length == 2) || fail(11);
        test((y = "ale", y += "xandre").length == 9) || fail(12);
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
        test(true + true == 2) || fail(27);
        test(1 + true == true + 1) || fail(28);
        test(true + false == true) || fail(29);
        test(true.plus(2) == 2.plus(true)) || fail(30);
        test(true + true == !false + true) || fail(31);
        test(true + !!!false == !false + true) || fail(32);
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
        test(String.getLength("test") == 4) || fail(10);
        test(String("test") == "test") || fail(11);
        test(Number(4) == 4) || fail(12);
        test(Number("4.3") == 4.3) || fail(13);
        test(String(5) == "5") || fail(14);
        test(Boolean(true && false) == false) || fail(15);
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
        test(this(this)["this"](this)["value"] == value) || fail(10);
        test(this["this"]["value"] == value) || fail(11);
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
        test((this["value"]++, this["value"]) == 7) || fail(6);
        test(this["value"]++ == 7) || fail(7);
        test(this["value"]-- == 8) || fail(8);
        test(this(this)["this"](this)["this"]["this"]["value"] == value) || fail(9);
        test(this["value"] && this.get("value")) || fail(10);
        test(value == this["value"]) || fail(11);
        test(this["self"]["property"] == this["property"]) || fail(12);
        test(this.value == value) || fail(13);
        test((this.value = 5, value == 5)) || fail(14);
        test((this.value = 'b') && value == 'b') || fail(15);
        test(this.self == this && this.self == getSelf()) || fail(16);
        test(this.self["property"] == this["property"]) || fail(17);
        test((this.self["value"] = 7, value == 7)) || fail(18);
        test(this.self.value++ == 7 && value == 8) || fail(19);
        test((this.self.self.self["this"].self.value--, this.value == 7)) || fail(20);
        test((this.self["value"]++) && this.getValue() == 8) || fail(21);
        test((this.setValue(this.getValue() + 1), this.value == 9) && (this.value *= 2, this.value == 18)) || fail(22);
        test((this.value += 5) == 23) || fail(23);
        end();
    }

    fun array()
    {
        // TODO
    }

    fun dictionary()
    {
        begin("Dictionary");

        weight = spawn("Dictionary");
        weight["Surge"] = 35.0;
        weight["Neon"] = 25.0;
        weight["Charge"] = 37.5;

        test(weight["Surge"] == 35.0) || fail(1);
        test(weight["Neon"] == 25.0) || fail(2);
        test(weight["Charge"] > 37 && weight["Charge"] < 38) || fail(3);
        test(weight.has("Surge")) || fail(4);
        test(!weight.has("Gimacian")) || fail(5);

        it = weight.iterator();
        test(it != null) || fail(6);
        test(it.hasNext()) || fail(7);

        sum = 0.0;
        while(it.hasNext()) {
            sum += weight[it.item];
            it.next();
        }
        test(sum > 97 && sum < 98) || fail(8);

        weight["Neon"] = 26;
        test(weight["Neon"] == 26) || fail(9);
        test(weight.has("Neon")) || fail(10);

        test(weight.size == 3) || fail(11);
        weight.delete("Neon");
        weight.delete("404");
        weight.delete(404);
        test(weight.has("Surge")) || fail(12);
        test(!weight.has("Neon")) || fail(13);
        test(weight.has("Charge")) || fail(14);
        test(weight.size == 2) || fail(15);

        weight.clear();
        test(weight.size == 0) || fail(16);
        test(!weight.iterator().hasNext() && null == weight.iterator().item) || fail(17);

        stress = spawn("Dictionary");
        stressLimit = 1000;
        items = [];
        for(j = 1; j <= stressLimit; j++)
            items.push(j);
        for(x in items)
            stress["o" + items[x]] = items[x];
        items.shuffle();
        test(stress["o500"] == 500) || fail(18);
        test(stress["o657"] == 657) || fail(19);
        test(!stress.has("o0") && stress.has("o700")) || fail(20);
        for(sum = 0, it = stress.iterator(); it.hasNext(); sum += stress[it.next()]);
        test(stress.size == stressLimit) || fail(21);
        test(sum == 500500) || fail(22);

        del = [ 1, 812, 726, 111, 182, 313, 6, 7, 8, 10, 88 ];
        newSize = stressLimit - del.length;
        test23 = true;
        for(x in del) {
            stress.delete("o" + del[x]);
            test23 = test23 && !stress.has("o" + del[x]);
        }
        test(test23) || fail(23);
        test(stress.size == newSize) || fail(24);

        del = [ -5, 0, -2, 31415, 12345, "a" ];
        test25 = true;
        for(x in del) {
            stress.delete("o" + del[x]), stress.delete(del[x]);
            test25 = test25 && !stress.has("o" + del[x]) && !stress.has(del[x]);
        }
        test(test25) || fail(25);
        test(stress.size == newSize) || fail(26);

        del = [];
        for(j = 1; j <= stressLimit; j++)
            del.push(j);
        for(x in del)
            stress.delete("o" + del[x]);
        test(stress.size == 0) || fail(27);

        end();
    }

    fun session()
    {
        begin("Session variables");
        test(Application.session != null) || fail(1);
        
        Application.session["lives"] = 5;
        test(Application.session["lives"] == 5) || fail(2);

        Application.session["name"] = "alex";
        test(Application.session["name"] == "alex") || fail(3);

        test(Application.session.size == 2) || fail(4);

        Application.session.clear();
        test(Application.session.size == 0) || fail(5);
        end();
    }





    // __constructor()
    // called automatically by SurgeScript when the object is created
    fun __constructor()
    {
        Console.print("------------------------------");
        Console.print("   SurgeScript Unit Testing   ");
        Console.print("------------------------------");
    }

    // __destructor()
    // called automatically by SurgeScript when the object is destroyed
    fun __destructor()
    {
        str = "\n";
        str += "SUCCEEDED  " + (totalTested - totalFailed) + "\t\t";
        str += "FAILED  " + totalFailed;
        Console.print(str);
    }

    // test(expr)
    // tests an expression; returns the value of the expression
    fun test(expr)
    {
        tested++;
        return expr;
    }

    // fail(x)
    // fails a test
    fun fail(x)
    {
        Console.print("# Test " + x + " has failed.");
        failed++;
    }

    // begin(suite)
    // begins a test suite
    fun begin(suite)
    {
        Console.print("\nTesting " + suite + "...");
        tested = 0;
        failed = 0;
    }

    // end()
    // ends a test suite
    fun end()
    {
        if(!failed)
            Console.print("All tests have passed.");
        totalTested += tested;
        totalFailed += failed;
    }

    // call()
    // calling object(expr) is the same as calling object.call(expr)
    fun call(expr)
    {
        return expr;
    }

    // get(key)
    // calling object["key"] is the same as calling object.get("key")
    fun get(key)
    {
        if(key == "this" || key == "self")
            return this;
        else if(key == "value")
            return value;
        else
            return null;
    }

    // set(key, newValue)
    // object[key] = newValue is the same as object.set(key, newValue) [also known as syntax sugar]
    fun set(key, newValue)
    {
        if(key == "value") // aka "public property"
            value = newValue;

        return this;    
    }

    // getValue()
    // object.value returns the same as object.getValue() [synax sugar]
    fun getValue()
    {
        return value;
    }

    // setValue(newValue)
    // object.value = newValue calls object.setValue(newValue) behind the scenes
    fun setValue(newValue)
    {
        value = newValue;
    }

    // getSelf()
    // We declare this function so that the expression "this.self" returns this (thus enabling some interesting test cases)
    fun getSelf()
    {
        return this;
    }
}