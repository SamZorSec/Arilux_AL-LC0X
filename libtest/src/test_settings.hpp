#include <catch2/catch.hpp>
#include <stdint.h>
#include <iostream>
#include <settings.h>

#ifndef millis
uint32_t millisStubbed = 0;
uint32_t millis() {
    return millisStubbed;
};
#endif

/**
 * Test if request for modification gets called
 */
void should_shouldCallModifiedFunction() {
    millisStubbed = 0;
    bool calledModified = false;
    Settings settings(
        200,
        1000, 
        []() {},
        [&calledModified]() {calledModified = true;return false;}
    );

    TEST_ASSERT_FALSE(calledModified);
    settings.handle();        
    TEST_ASSERT_TRUE(calledModified);
}

/**
 * Tests that we never call save if nothing was modified
 */ 
void should_testDontStoreIfNotModified() {
    millisStubbed = 0;
    bool calledSaved = false;
    Settings settings(
        200,
        1000, 
        [&calledSaved]() {calledSaved=true;},
        []() {return false;}
    );

    // Repeated calls to handle will not store 
    for (int i=0;i<2000;i+=100) {
        millisStubbed=i;
        settings.handle();        
        TEST_ASSERT_FALSE(calledSaved);
    }
}

/**
 * Test that we wil call save but only after debounce time
 */
void should_storeAfterDebounceButWaitMinTime() {
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
    TEST_ASSERT_FALSE(calledSaved);
    modified = false; 

    // at 200 debounce is over and should save 
    millisStubbed=200;
    settings.handle();
    TEST_ASSERT_TRUE(calledSaved);
    calledSaved=false;

    // At 500ms modified again, should not save
    modified = true; 
    millisStubbed=500;
    settings.handle();
    TEST_ASSERT_FALSE(calledSaved);
    modified = false; 

    // at 800ms still modified, but we wait for min wait time
    modified = true; 
    millisStubbed=800;
    settings.handle();
    TEST_ASSERT_FALSE(calledSaved);
    modified = false; 

    // at 1200ms we will save as min wait time is over
    millisStubbed=1200;
    settings.handle();
    TEST_ASSERT_TRUE(calledSaved);
    calledSaved = false;

    // try againmin wait time
    modified = true; 
    millisStubbed=1600;
    settings.handle();
    modified = false;
    millisStubbed=2000;
    settings.handle();
    TEST_ASSERT_FALSE(calledSaved);
    millisStubbed=2200;
    settings.handle();
    TEST_ASSERT_TRUE(calledSaved);
}

void should_shouldSaveAtForce() {
        millisStubbed = 0;
    bool calledSaved = false;
    Settings settings(
        200,
        1000, 
        [&calledSaved]() {calledSaved = true;},
        []() {return false;}
    );

    TEST_ASSERT_FALSE(calledSaved);
    settings.save(true);        
    TEST_ASSERT_TRUE(calledSaved);
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

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(should_shouldCallModifiedFunction);
    RUN_TEST(should_testDontStoreIfNotModified);
    RUN_TEST(should_storeAfterDebounceButWaitMinTime);
    RUN_TEST(should_shouldSaveAtForce);
    UNITY_END();

    show_showMinWaitTimeInAction();

    return 0;
}