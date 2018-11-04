#include <hsb.h>
#include <catch2/catch.hpp>

TEST_CASE( "hue can be", "[hsb]" ) {
    SECTION("fixed with ints") {
        REQUIRE(HSB::fixHue_int16_t(100) == 100);
        REQUIRE(HSB::fixHue_int16_t(400) == 40);
        REQUIRE(HSB::fixHue_int16_t(360) == 0);
        REQUIRE(HSB::fixHue_int16_t(0) == 0);
        REQUIRE(HSB::fixHue_int16_t(-40) == 320);
    }

    SECTION("fixed with floats") {
        REQUIRE(HSB::fixHue(100.0) == Approx(100.0));
        REQUIRE(HSB::fixHue(400.0) == Approx(40.0));
        REQUIRE(HSB::fixHue(360.0) == Approx(0.0));
        REQUIRE(HSB::fixHue(0.0) == Approx(0.0));
        REQUIRE(HSB::fixHue(-40.0) == Approx(320.0));
    }

    SECTION("calculated to shortest path") {
        REQUIRE(HSB::hueShortestPath(0.0, 90.0) == Approx(90.0));
        REQUIRE(HSB::hueShortestPath(340.0, 40.0) == Approx(400.0));
        REQUIRE(HSB::hueShortestPath(0.0, 181.0) == Approx(-179.0));
        REQUIRE(HSB::hueShortestPath(240.0, 0.0) == Approx(360));
        REQUIRE(HSB::hueShortestPath(10.0, 340.0) == Approx(-20.0));
    }
}

TEST_CASE( "get HSB values", "[hsb]" ) {

     SECTION( "get HSB and w1,w2" ) {
        HSB hsb(1.0, 2.0, 3.0, 4.0, 5.0);
        REQUIRE(hsb.hue() == Approx( 1.0));
        REQUIRE(hsb.saturation() == Approx( 2.0));
        REQUIRE(hsb.brightness() == Approx( 3.0));
        REQUIRE(hsb.white1() == Approx( 4.0));
        REQUIRE(hsb.white2() == Approx( 5.0));
    }
    
     SECTION( "get HSB as array" ) {
        HSB hsb(1.0, 2.0, 3.0, 4.0, 5.0);
        float colors[3];
        hsb.getHSB(colors);
        REQUIRE(colors[0] == Approx( 1.0));
        REQUIRE(colors[1] == Approx( 2.0));
        REQUIRE(colors[2] == Approx( 3.0));
    }

}    
    
TEST_CASE( "get RGB values", "[hsb]" ) {
    SECTION("get RGB with HSB red brightness 100%") {
        float colors[3];
        HSB hsb(0.0, 100.0, 100.0, 0.0, 0.0);
        hsb.constantRGB(colors);
        REQUIRE(colors[0] == Approx(100.0));
        REQUIRE(colors[1] == Approx(0.0));
        REQUIRE(colors[2] == Approx(0.0));       
    }

    SECTION("get RGB with HSB green brightness 50%") {
        float colors[3];
        HSB hsb(120.0, 100.0, 50.0, 0.0, 0.0);
        hsb.constantRGB(colors);
        REQUIRE(colors[0] == Approx(0.0));    
        REQUIRE(colors[1] == Approx(50.0));    
        REQUIRE(colors[2] == Approx(0.0));         
    }

    SECTION("get RGB with HSB blue 100% saturation 50%") {
        float colors[3];
        HSB hsb(240.0, 50.0, 100.0, 0.0, 0.0);
        hsb.constantRGB(colors);
        REQUIRE(colors[0] == Approx(50.0));
        REQUIRE(colors[1] == Approx(50.0));
        REQUIRE(colors[2] == Approx(100.0));     
    }

}
 