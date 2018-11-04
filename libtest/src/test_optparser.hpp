#include <optparser.h>
#include <catch2/catch.hpp>


TEST_CASE( "Should get floats and ints", "[optparser]" ) {
   int calls=0;
    OptParser::get("0.1,100.124,-100.678,-100,200", ",", [&calls](OptValue f) {
        if (f.pos()==0) {
            REQUIRE(f.asFloat() == Approx(0.1));
            calls++;
        }
        if (f.pos()==1) {
            REQUIRE(f.asFloat() == Approx(100.124));
            calls++;
        }
        if (f.pos()==2) {
            REQUIRE(f.asFloat() == Approx(-100.678));
            calls++;
        }
        if (f.pos()==3) {
            REQUIRE(f.asInt() == -100);
            calls++;
        }
        if (f.pos()==4) {
            REQUIRE(f.asInt() == 200);
            calls++;
        }
    });
    REQUIRE(calls == 5);
}

TEST_CASE( "Should get ints from floats", "[optparser]" ) {
    int calls=0;
    OptParser::get("abc ,  567.456,  -3567.8521  ",  ",", [&calls](OptValue f) {
        if (f.pos()==1) {
            REQUIRE(f.asInt() == 567);
            calls++;
        }
        if (f.pos()==2) {
            REQUIRE(f.asInt() == -3567);
            calls++;
        }
    });
    REQUIRE(calls == 2);
}

TEST_CASE( "Should call with one item", "[optparser]" ) {
    int calls=0;
    OptParser::get("oneitem",  ",", [&calls](OptValue f) {
        REQUIRE( std::strcmp( f.key(), "oneitem" ) == 0 );
        calls++;
    });
    REQUIRE(calls == 1);
}

TEST_CASE( "Should not call when empty", "[optparser]" ) {
    int calls=0;
    OptParser::get("",  ",", [&calls](OptValue f) {
            calls++;
    });
    REQUIRE(calls == 0);
}

TEST_CASE( "Should handle variabel names", "[optparser]" ) {
    int calls=0;
    OptParser::get("abc=1,2,3     w =400.123 xyz=abc   ", [&calls](OptValue f) {
        if (f.pos()==0) {
            REQUIRE( std::strcmp( f.key(), "abc" ) == 0 );
            REQUIRE( std::strcmp( f.asChar(), "1,2,3" ) == 0 );
            calls++;
        }
        if (f.pos()==1) {
            REQUIRE( std::strcmp( f.key(), "w" ) == 0 );
            REQUIRE(f.asFloat() == Approx(400.123));
            calls++;
        }
        if (f.pos()==2) {
            REQUIRE( std::strcmp( f.key(), "xyz" ) == 0 );
            REQUIRE( std::strcmp( f.asChar(), "abc" ) == 0 );
            calls++;
        }
    });
    REQUIRE(calls == 3);
}

TEST_CASE( "Should handle variabel strings with spaces", "[optparser]" ) {
    int calls=0;
    OptParser::get("str1= Hello  there ,str2 = Some other string     ", ",", [&calls](OptValue f) {
        if (f.pos()==0) {
            REQUIRE( std::strcmp( f.key(), "str1" ) == 0 );
            REQUIRE( std::strcmp( f.asChar(), "Hello  there " ) == 0 );
            calls++;
        }
        if (f.pos()==1) {
            REQUIRE( std::strcmp( f.key(), "str2" ) == 0 );
            REQUIRE( std::strcmp( f.asChar(), "Some other string" ) == 0 );
            calls++;
        }
        
    });
    REQUIRE(calls == 2);
}
