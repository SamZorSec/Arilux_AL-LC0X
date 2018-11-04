#include <catch2/catch.hpp>
#include <stdint.h>
#include <iostream>
#include <settings.h>
#include "arduinostubs.hpp"

TEST_CASE( "Should check for modifications", "[settings]" ) {
    millisStubbed = 0;
    bool calledModified = false;
    Settings settings(
        200,
        1000, 
        []() {},
        [&calledModified]() {calledModified = true;return false;}
    );

    REQUIRE(calledModified == false);
    settings.handle();        
    REQUIRE(calledModified == true);

}

TEST_CASE( "Should never call save function when not modified", "[settings]" ) {
    millisStubbed = 0;
    bool calledSaved = false;
    Settings settings(
        200,
        1000, 
        [&calledSaved]() {calledSaved=true;},
        []() {return false;}
    );

    // Repeated calls to handle test with timings
    for (int i=0;i<2000;i+=100) {
        millisStubbed=i;
        settings.handle();        
        REQUIRE(calledSaved == false);
    }
}

TEST_CASE( "Should call save after debounce", "[settings]" ) {
     millisStubbed = 0;
    bool modified = false;
    bool calledSaved = false;
    Settings settings(
        200,
        1000, 
        [&calledSaved]() {calledSaved=true;},
        [&modified]() {return modified;}
    );

    settings.handle();       

    // Modified at 195ms
    modified = true; 
    millisStubbed=195;
    settings.handle();       
    REQUIRE(calledSaved == false);
    modified = false; 

    // at 200 debounce is over and should save 
    millisStubbed=200;
    settings.handle();
    REQUIRE(calledSaved == true);
    calledSaved=false;

    // At 500ms modified again, should not save
    modified = true; 
    millisStubbed=500;
    settings.handle();
    REQUIRE(calledSaved == false);
    modified = false; 

    // at 800ms still modified, but we wait for min wait time
    modified = true; 
    millisStubbed=800;
    settings.handle();
    REQUIRE(calledSaved == false);
    modified = false; 

    // at 1200ms we will save as min wait time is over
    millisStubbed=1200;
    settings.handle();
    REQUIRE(calledSaved == true);
    calledSaved = false;

    // try againmin wait time
    modified = true; 
    millisStubbed=1600;
    settings.handle();
    modified = false;
    millisStubbed=2000;
    settings.handle();
    REQUIRE(calledSaved == false);
    millisStubbed=2200;
    settings.handle();
    REQUIRE(calledSaved == true);
}

TEST_CASE( "Should call save when we force", "[settings]" ) {
        millisStubbed = 0;
    bool calledSaved = false;
    Settings settings(
        200,
        1000, 
        [&calledSaved]() {calledSaved = true;},
        []() {return false;}
    );

    REQUIRE(calledSaved == false);
    settings.save(true);        
    REQUIRE(calledSaved == true);
}

void show_showMinWaitTimeInAction(){
    millisStubbed = 0;
    bool calledSaved = false;
    Settings settings(
        1,
        1000, 
        [&calledSaved]() {std::cerr << " saved";},
        [&calledSaved]() {calledSaved=false;return true;}
    );

    for (int i=0;i<10000;i+=50) {
        std::cerr << i;
        millisStubbed = i;
        settings.handle();
        std::cerr << "\n";
    }
}
