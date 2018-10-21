#include <unity.h>
#include <optparser.h>

// Should correctly get float and int values
void shouldGetFloatsAndInts() {
    int calls=0;
    OptParser::get("0.1,100.124,-100.678,-100,200", ",", [&calls](OptValue f) {
        if (f.pos()==0) {
            TEST_ASSERT_EQUAL_FLOAT(0.1, f.asFloat());
            calls++;
        }
        if (f.pos()==1) {
            TEST_ASSERT_EQUAL_FLOAT(100.124, f.asFloat());
            calls++;
        }
        if (f.pos()==2) {
            TEST_ASSERT_EQUAL_FLOAT(-100.678, f.asFloat());
            calls++;
        }
        if (f.pos()==3) {
            TEST_ASSERT_EQUAL_INT(-100, f.asInt());
            calls++;
        }
        if (f.pos()==4) {
            TEST_ASSERT_EQUAL_INT(200, f.asInt());
            calls++;
        }
    });
    TEST_ASSERT_EQUAL_INT(5, calls);
}

// Should get a int value even though a float value was givej
void shouldGetIntsFromFloats() {
    int calls=0;
    OptParser::get("abc ,  567.456,  -3567.8521  ",  ",", [&calls](OptValue f) {
        if (f.pos()==1) {
            TEST_ASSERT_EQUAL_INT(567, f.asInt());
            calls++;
        }
        if (f.pos()==2) {
            TEST_ASSERT_EQUAL_INT(-3567, f.asInt());
            calls++;
        }
    });
    TEST_ASSERT_EQUAL_INT(2, calls);
}

// Should handle assignment types
void shouldHandleAssignments() {
    int calls=0;
    OptParser::get("abc=1,2,3     w =400.123 xyz=abc   ", [&calls](OptValue f) {
        if (f.pos()==0) {
            TEST_ASSERT_EQUAL_STRING("abc", f.key());
            TEST_ASSERT_EQUAL_STRING("1,2,3", f.asChar());
            calls++;
        }
        if (f.pos()==1) {
            TEST_ASSERT_EQUAL_STRING("w", f.key());
            TEST_ASSERT_EQUAL_FLOAT(400.123, f.asFloat());
            calls++;
        }
        if (f.pos()==2) {
            TEST_ASSERT_EQUAL_STRING("abc", f.asChar());
            TEST_ASSERT_EQUAL_STRING("xyz", f.key());
            calls++;
        }
    });
    TEST_ASSERT_EQUAL_INT(3, calls);
}

// Should handle assignment types
void shouldHandleStrings() {
    int calls=0;
    OptParser::get("str1= Hello  there ,str2 = Some other string     ", ",", [&calls](OptValue f) {
        if (f.pos()==0) {
            TEST_ASSERT_EQUAL_STRING("str1", f.key());
            TEST_ASSERT_EQUAL_STRING("Hello  there ", f.asChar());
            calls++;
        }
        if (f.pos()==1) {
            TEST_ASSERT_EQUAL_STRING("str2", f.key());
            TEST_ASSERT_EQUAL_STRING("Some other string", f.asChar());
            calls++;
        }
        
    });
    TEST_ASSERT_EQUAL_INT(2, calls);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(shouldGetFloatsAndInts);
    RUN_TEST(shouldGetIntsFromFloats);
    RUN_TEST(shouldHandleAssignments);
    RUN_TEST(shouldHandleStrings);
    UNITY_END();

    return 0;
}