integer gTestsPassed = 0;
integer gTestsFailed = 0;

testPassed(string description, string actual, string expected)
{
    ++gTestsPassed;
    //llSay(0, description);
}

testFailed(string description, string actual, string expected)
{
    ++gTestsFailed;
    print("FAILED!: " + description + " (" + actual + " expected " + expected + ")");
    sdAssert(0);
}

ensureTrue(string description, integer actual)
{
    if(actual)
    {
        testPassed(description, (string) actual, (string) TRUE);
    }
    else
    {
        testFailed(description, (string) actual, (string) TRUE);
    }
}

ensureFalse(string description, integer actual)
{
    if(actual)
    {
        testFailed(description, (string) actual, (string) FALSE);
    }
    else
    {
        testPassed(description, (string) actual, (string) FALSE);
    }
}

ensureIntegerEqual(string description, integer actual, integer expected)
{
    if(actual == expected)
    {
        testPassed(description, (string) actual, (string) expected);
    }
    else
    {
        testFailed(description, (string) actual, (string) expected);
    }
}

integer floatEqual(float actual, float expected)
{
    float error = llFabs(expected - actual);
    float epsilon = 0.001;
    if(error > epsilon)
    {
        print("Float equality delta " + (string)error);
        return FALSE;
    }
    return TRUE;
}

ensureFloatEqual(string description, float actual, float expected)
{
    if(floatEqual(actual, expected))
    {
        testPassed(description, (string) actual, (string) expected);
    }
    else
    {
        testFailed(description, (string) actual, (string) expected);
    }
}

ensureStringEqual(string description, string actual, string expected)
{
    if(actual == expected)
    {
        testPassed(description, (string) actual, (string) expected);
    }
    else
    {
        testFailed(description, (string) actual, (string) expected);
    }
}

ensureVectorEqual(string description, vector actual, vector expected)
{
    if(floatEqual(actual.x, expected.x) &&
        floatEqual(actual.y, expected.y) &&
        floatEqual(actual.z, expected.z))
    {
        testPassed(description, (string) actual, (string) expected);
    }
    else
    {
        testFailed(description, (string) actual, (string) expected);
    }
}

ensureRotationEqual(string description, rotation actual, rotation expected)
{
    if(floatEqual(actual.x, expected.x) &&
        floatEqual(actual.y, expected.y) &&
        floatEqual(actual.z, expected.z) &&
        floatEqual(actual.s, expected.s))
    {
        testPassed(description, (string) actual, (string) expected);
    }
    else
    {
        testFailed(description, (string) actual, (string) expected);
    }
}

ensureListEqual(string description, list actual, list expected)
{
    // equal in length and actual contains expected (== is just length comparison)
    if(actual == expected && ((string)actual == (string)expected))
    {
        testPassed(description, (string) actual, (string) expected);
    }
    else
    {
        testFailed(description, (string) actual, (string) expected);
    }
}

integer gInteger = 5;
float gFloat = 1.5;
string gString = "foo";
vector gVector = <1, 2, 3>;
rotation gRot = <1, 2, 3, 4>;
list gList = [1, 2, 3];
list gCallOrder;

integer callOrderFunc(integer num) {
    gCallOrder += [num];
    return 1;
}

integer testReturn()
{
    return 1;
}

float testReturnFloat()
{
    return 1.0;
}

string testReturnString()
{
    return "Test string";
}

list testReturnList()
{
    return [1,2,3];
}

vector testReturnVector()
{
    return <1,2,3>;
}

rotation testReturnRotation()
{
    return <1,2,3,4>;
}

vector testReturnVectorNested()
{
    return testReturnVector();
}

vector testReturnVectorWithLibraryCall()
{
    llSin(0);
    return <1,2,3>;
}

rotation testReturnRotationWithLibraryCall()
{
    llSin(0);
    return <1,2,3,4>;
}

integer testParameters(integer param)
{
    param = param + 1;
    return param;
}

integer testRecursion(integer param)
{
    if(param <= 0)
    {
        return 0;
    }
    else
    {
        return testRecursion(param - 1);
    }
}

string testExpressionLists(list l)
{
    return "foo" + (string)l;
}

tests()
{
    ensureStringEqual("List to string cast", (string)[5,4,3,2,"foo"], "5432foo");
    vector vectest = <1,2,3>;
    list veclist = [vectest];
    vectest.x = 2.0;
    ensureStringEqual("Vector references not shared1", (string)veclist, "<1.000000, 2.000000, 3.000000>");
    // what? decimal place difference in precision between the two casts?
    ensureStringEqual("Vector references not shared2", (string)vectest, "<2.00000, 2.00000, 3.00000>");
}

runTests()
{
    tests();
    print("All tests passed");
    // reset globals
    gInteger = 5;
    gFloat = 1.5;
    gString = "foo";
    gVector = <1, 2, 3>;
    gRot = <1, 2, 3, 4>;
    gList = [1, 2, 3];
    gTestsPassed = 0;
    gTestsFailed = 0;
    gCallOrder = [];
}

default
{
    state_entry()
    {
        runTests();
    }
}