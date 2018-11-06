#include <colorseffect.h>
#include <catch2/catch.hpp>
#include <hsb.h>

SCENARIO( "Colorseffect will change based on time", "[colors]" ) {
    GIVEN( "A the object gets initialised with no colors" ) {

        ColorsEffect colorseffect({}, 1000, 0);
        WHEN( "we are at time 1500" ) {
            THEN( "should return given color" ) {
                HSB hsb = colorseffect.handleEffect(0, 1500, HSB(20.0f, 0.0f,  0.0f,  0.0f,  0.0f));
                REQUIRE(hsb.hue() == Approx(20.0f));
            }
        }
    }

    GIVEN( "A the object gets initialised with 3 colors with start time 0" ) {

        ColorsEffect colorseffect({0,120,240}, 1000, 0);
        WHEN( "we are at time 0" ) {
            THEN( "Color at time 0 should be 0" ) {
                HSB hsb = colorseffect.handleEffect(0, 0, HSB(20.0f, 0.0f,  0.0f,  0.0f,  0.0f));
                REQUIRE(hsb.hue() == Approx(0.0f));
            }
            THEN( "Color at time 250 should be 0" ) {
                HSB hsb = colorseffect.handleEffect(0, 250, HSB(20.0f, 0.0f,  0.0f,  0.0f,  0.0f));
                REQUIRE(hsb.hue() == Approx(0.0f));
            }
            THEN( "Color at time 750 should be 0" ) {
                HSB hsb = colorseffect.handleEffect(0, 750, HSB(20.0f, 0.0f,  0.0f,  0.0f,  0.0f));
                REQUIRE(hsb.hue() == Approx(0.0f));
            }
            THEN( "Color at time 1750 should be 120" ) {
                HSB hsb = colorseffect.handleEffect(0, 1750, HSB(20.0f, 0.0f,  0.0f,  0.0f,  0.0f));
                REQUIRE(hsb.hue() == Approx(120.0f));
            }
            THEN( "Color at time 2750 should be 120" ) {
                HSB hsb = colorseffect.handleEffect(0, 2750, HSB(20.0f, 0.0f,  0.0f,  0.0f,  0.0f));
                REQUIRE(hsb.hue() == Approx(240.0f));
            }
            THEN( "Color at time 3750 should be 0" ) {
                HSB hsb = colorseffect.handleEffect(0, 3750, HSB(20.0f, 0.0f,  0.0f,  0.0f,  0.0f));
                REQUIRE(hsb.hue() == Approx(0.0f));
            }
        }
    }

    GIVEN( "A the object gets initialised with 3 colors with start time 10200" ) {
        ColorsEffect colorseffect({0,120,240}, 1000, 10200);
        WHEN( "we are at time 11300" ) {
            THEN( "Color at time 10300 should be 120" ) {
                HSB hsb = colorseffect.handleEffect(0, 11300, HSB(20.0f, 0.0f,  0.0f,  0.0f,  0.0f));
                REQUIRE(hsb.hue() == Approx(120.0f));
            }        
        }
    }

}