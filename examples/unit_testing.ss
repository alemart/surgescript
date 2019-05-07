//
// unit_testing.ss
// A Unit Testing Script for SurgeScript
// Copyright (C) 2017-2018 Alexandre Martins <alemartf(at)gmail(dot)com>
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
        test.functor();
        test.getset();
        test.array();
        test.dictionary();
        exit();
    }
}

object "SurgeScriptTest"
{
    public message = "Amazing!";
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
        test(Number("37") == 37) || fail(8);
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
        test("test".substr(-1, 9999) == "t") || fail(22);
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
        test("" + this == this.toString()) || fail(33);
        test(toString() + 2 === this + "" + 2) || fail(34);
        test(toString().equals(this.toString())) || fail(35);
        test("".equals("") && !"a".equals("b")) || fail(36);
        test("23".equals("2" + "3") && !"23".equals(23)) || fail(37);
        test((typeof this).equals("object")) || fail(38);
        test("Surge, Neon and Charge".replace("Surge", "Gimacian") == "Gimacian, Neon and Charge") || fail(39);
        test("Surge, Neon and Charge".replace("Neon", "Gimacian") == "Surge, Gimacian and Charge") || fail(40);
        test("Surge, Neon and Charge".replace("Charge", "Gimacian") == "Surge, Neon and Gimacian") || fail(41);
        test("Surge, Neon and Charge".replace("", "Gimacian") == "Surge, Neon and Charge") || fail(42);
        test("Surge, Neon and Charge".replace("Gimacian", "") == "Surge, Neon and Charge") || fail(43);
        test("SurgeScript".toLowerCase() == "surgescript") || fail(44);
        test("SurgeScript".toUpperCase() == "SURGESCRIPT") || fail(45);
        test("1" !== 1) || fail(46);
        test("0" !== null) || fail(47);
        test(null === null) || fail(48);
        test(0 !== "0" && 0 == "0") || fail(49);
        test(state == "main") || fail(50);
        test(state != null) || fail(51);
        test("test".substr(4, 1) == "" && "test".substr(-1, 1) == "t") || fail(52);
        test("test".substr(3, 50) == "t") || fail(53);
        end();
    }

    fun number()
    {
        begin("Number");
        test(1 == 1) || fail(1);
        test(2 + 2 == 4) || fail(2);
        test((3 * 3).toString() == "9") || fail(3);
        test(Number(4.toString()) == 4.valueOf()) || fail(4);
        test(Number(5 * "25") == 625 / 5) || fail(5);
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
        test(1.equals(1) && !2.equals("2")) || fail(28);
        test(5 % 3 == 2) || fail(29);
        test(-5 % 3 == -2) || fail(30);
        test(5 % -3 == 2) || fail(31);
        test(-5 % -3 == -2) || fail(32);
        test(0 % 1 == 0) || fail(33);
        test(-0 % 1 == 0) || fail(34);
        test((5 % 0).isNaN()) || fail(35);
        test((5 % Math.NaN).isNaN()) || fail(36);
        test((Math.infinity % 2).isNaN()) || fail(37);
        test((-5 % Math.infinity).equals(-5)) || fail(38);
        test(5 + 5 % 3 == 7) || fail(39);
        test(5 + 5 % -3 * -2 == 1) || fail(40);
        test((1 == null) === false) || fail(41);
        test((1 === null) === false) || fail(42);
        test((1 == 2) === false) || fail(43);
        test((1 === 2) === false) || fail(44);
        test((1 != null) === true) || fail(45);
        test((1 !== null) === true) || fail(46);       
        test((1 != 2) === true) || fail(47);
        test((1 !== 2) === true) || fail(48);
        test(1 < Math.infinity) || fail(49);
        test(9999 <= Math.infinity) || fail(50);
        test(-1 > -Math.infinity) || fail(51);
        test(-9999 >= -Math.infinity) || fail(52);
        test(Math.pi != Math.infinity) || fail(53);
        test(1 < 2 * Math.infinity) || fail(54);
        test(1 > -2 * Math.infinity) || fail(55);
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
        test(true && true === true) || fail(13);
        test(true && false === false) || fail(14);
        test(false || true === true) || fail(15);
        test(false || false === false) || fail(16);
        test(false && true || true === true) || fail(17);
        test((false && (true || true)) === false) || fail(18);
        test((!true && !false || !false) === true) || fail(19);
        test((!true && (!false || !false)) === false) || fail(20);
        test((!true && !(false && false)) === false) || fail(21);
        test((!true && !false) === !(true || false)) || fail(22);
        test((!true && !!false) === !(true || !false)) || fail(23);
        test(!true === false) || fail(24);
        test(true === !false) || fail(25);
        test(!(true != false) === (true == false)) || fail(26);
        test(true + true == 2 * true) || fail(27);
        test(1 + true == true + 1) || fail(28);
        test(true + false == true) || fail(29);
        test(true + 2 == !false + 2) || fail(30);
        test(true + true == !false + true) || fail(31);
        test(true + !!!false == !false + true) || fail(32);
        test(true.equals(!false) && (!true).equals(false)) || fail(33);
        test(!(true.equals(true && !(true.equals(true))))) || fail(34);
        end();
    }

    fun objects()
    {
        begin("Object");
        test(typeof this == "object") || fail(1);
        test(this.__name == "SurgeScriptTest") || fail(2);
        test(Application != null && Application.__name == "Application") || fail(3);
        test(this != Application) || fail(4);
        test(this.toString() == toString()) || fail(5);
        test(this.hasFunction("objects")) || fail(6);
        test(!this.hasFunction("nope")) || fail(7);
        test(findObject("nope") == null) || fail(8);
        test((c = spawn("Boolean"), c != null && child("Boolean") == c)) || fail(9);
        test((d = spawn("Number"), d != null && d == findObject("Number"))) || fail(10);
        test(typeof null == "null") || fail(11);
        test(!this.equals(Application) && this.equals(this)) || fail(12);
        test(null !== this) || fail(13);
        test(null === this.child("foo")) || fail(14);
        end();
    }

    fun system()
    {
        begin("System objects");
        test(Application) || fail(1);
        test(Application.__name == "Application") || fail(2);
        test(Application.__name != String.__name) || fail(3);
        test((Application = "foo", Application != "foo")) || fail(4); // read-only
        test("foo" != String) || fail(5);
        test(String.__name == "String") || fail(6);
        test(Number.__name == "Number") || fail(7);
        test(Boolean.__name == "Boolean") || fail(8);
        test(String = null || String) || fail(9);
        test(String.get_length("test") == 4) || fail(10);
        test(String("test") == "test") || fail(11);
        test(Number(4) == 4) || fail(12);
        test(Number("4.3") == 4.3) || fail(13);
        test(String(5) == "5") || fail(14);
        test(Boolean(true && false) == false) || fail(15);
        test(String.destroy() == null && String.__name) || fail(16);
        end();
    }

    fun functor()
    {
        begin("Function objects");
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
        test(this.self == this && this.self == get_self()) || fail(16);
        test(this.self["property"] == this["property"]) || fail(17);
        test((this.self["value"] = 7, value == 7)) || fail(18);
        test(this.self.value++ == 7 && value == 8) || fail(19);
        test((this.self.self.self["this"].self.value--, this.value == 7)) || fail(20);
        test((this.self["value"]++) && this.get_value() == 8) || fail(21);
        test((this.set_value(this.get_value() + 1), this.value == 9) && (this.value *= 2, this.value == 18)) || fail(22);
        test((this.value += 5) == 23) || fail(23);
        end();
    }

    fun array()
    {
        begin("Array");
        test([].__name == "Array") || fail(1);
        test([] != []) || fail(2);
        test(spawn("Array").parent != [].parent) || fail(3);
        test([].parent == [].parent) || fail(4);
        test([1,2,3,4,5].length == 5) || fail(5);
        test([1,2,3][2] == 3) || fail(6);
        test((arr = [1,2,3], arr[0] = 5, arr[0] == 5)) || fail(7);
        test([1,2,3][-1] == null) || fail(8);
        test([1,2,3][5] == null) || fail(9);
        test((arr = [5,0,1], arr.push(7), arr.length == 4 && arr[3] == 7)) || fail(10);
        test(arr.pop() == 7 && arr.length == 3) || fail(11);
        test(arr.shift() == 5 && arr.length == 2 && arr[0] == 0 && arr[1] == 1) || fail(12);
        test((arr.unshift(3), arr[0] == 3 && arr.length == 3)) || fail(13);

        arr = [54,192,19,74,-1,292,123,124,122].sort(null);
        for(inOrder = true, i = 0; i < arr.length - 1 && inOrder; i++)
            inOrder = arr[i] < arr[i + 1];
        test(inOrder) || fail(14);

        arr = arr.reverse();
        for(inOrder = true, i = 0; i < arr.length - 1 && inOrder; i++)
            inOrder = arr[i] > arr[i + 1];
        test(inOrder) || fail(15);

        test([4,9,2,123,3,-1,0,4].indexOf(0) == 6) || fail(16);
        test([4,9,2,123,3,-1,0,4].indexOf('a') < 0) || fail(17);
        test([4,9,2,123,3,-1,0,4].indexOf(4) == 0) || fail(18);
        test([[],[],[5],[],[]].length == 5) || fail(19);

        arr = [1,2,3,4,5]; sum = 0.0;
        foreach(entry in arr)
            sum += entry;
        test(sum == 15) || fail(20);

        test([1,2,3,4,5].toString() == [5,4,3,2,1].reverse().toString()) || fail(21);
        test([1,2,3].toString().length == [1,1,1].toString().length) || fail(22);
        test([1,2,3,4,5].shuffle().sort(null).toString() == [4,2,5,3,1].shuffle().sort(null).toString()) || fail(23);
        test([1,2,3,4,5].shuffle().toString().length == [1,2,3,4,5].shuffle().toString().length) || fail(24);
        test([1,2,3,4,5].shuffle().length == 5) || fail(25);
        end();
    }

    fun dictionary()
    {
        begin("Dictionary");

        weight = {
            "Surge": 35.0,
            "Neon": 25.0,
            "Charge": 37.5
        };

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
            entry = it.next();
            sum += entry.value;
        }
        test(sum > 97 && sum < 98) || fail(8);

        weight["Neon"] = 26;
        test(weight["Neon"] == 26) || fail(9);
        test(weight.has("Neon")) || fail(10);

        test(weight.count == 3) || fail(11);
        weight.delete("Neon");
        weight.delete("404");
        weight.delete(404);
        test(weight.has("Surge")) || fail(12);
        test(!weight.has("Neon")) || fail(13);
        test(weight.has("Charge")) || fail(14);
        test(weight.count == 2) || fail(15);

        weight.clear();
        test(weight.count == 0) || fail(16);
        test(!weight.iterator().hasNext() && null == weight.iterator().next()) || fail(17);

        stress = spawn("Dictionary");
        stressLimit = 1000;
        items = [];
        for(j = 1; j <= stressLimit; j++)
            items.push(j);
        foreach(item in items)
            stress["o" + item] = item;
        items.shuffle();
        test(stress["o500"] == 500) || fail(18);
        test(stress["o657"] == 657) || fail(19);
        test(!stress.has("o0") && stress.has("o700")) || fail(20);
        for(sum = 0, it = stress.iterator(); it.hasNext(); sum += stress[it.next().key]);
        test(stress.count == stressLimit) || fail(21);
        test(sum == 500500) || fail(22);

        del = [ 1, 812, 726, 111, 182, 313, 6, 7, 8, 10, 88 ];
        newSize = stressLimit - del.length;
        test23 = true;
        foreach(item in del) {
            stress.delete("o" + item);
            test23 = test23 && !stress.has("o" + item);
        }
        test(test23) || fail(23);
        test(stress.count == newSize) || fail(24);

        del = [ -5, 0, -2, 31415, 12345, "a" ];
        test25 = true;
        for(x = 0; x < del.length; x++) {
            stress.delete("o" + del[x]), stress.delete(del[x]);
            test25 = test25 && !stress.has("o" + del[x]) && !stress.has(del[x]);
        }
        test(test25) || fail(25);
        test(stress.count == newSize) || fail(26);

        del = [];
        for(j = 1; j <= stressLimit; j++)
            del.push(j);
        foreach(item in del)
            stress.delete("o" + item);
        test(stress.count == 0) || fail(27);

        dict = { "a": 1, "b": 2, "c": 3 };
        test(dict["a"] + dict["b"] + dict["c"] == 6) || fail(28);
        it = dict.iterator();
        while(it.hasNext()) {
            entry = it.next();
            entry.value = 1;
        }
        test(dict["a"] + dict["b"] + dict["c"] == 3) || fail(29);
        for(keys = "", it = dict.iterator(); it.hasNext(); keys += it.next().key);
        test(keys == "abc") || fail(30);

        end();
    }




    // constructor()
    // called automatically by SurgeScript when the object is created
    fun constructor()
    {
        Console.print("------------------------------");
        Console.print("   SurgeScript Unit Testing   ");
        Console.print("------------------------------");
    }

    // destructor()
    // called automatically by SurgeScript when the object is destroyed
    fun destructor()
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
    // object[key] = newValue is the same as object.set(key, newValue) [syntactic sugar]
    fun set(key, newValue)
    {
        if(key == "value") // aka "public property"
            value = newValue;

        return this;    
    }

    // get_value()
    // object.value returns the same as object.get_value() [syntactic sugar]
    fun get_value()
    {
        return value;
    }

    // set_value(newValue)
    // object.value = newValue calls object.set_value(newValue) behind the scenes
    fun set_value(newValue)
    {
        value = newValue;
    }

    // get_self()
    // We declare this function so that the expression "this.self" returns this (thus enabling some interesting test cases)
    fun get_self()
    {
        return this;
    }
}
