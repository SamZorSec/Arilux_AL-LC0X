#include <unity.h>
#include <statemachine.h>
#include <iostream>

#ifndef millis
uint32_t millisStubbed = 0;
uint32_t millis() {
    return millisStubbed;
};
#endif

// Should correctly get current value with simple state machine
void should_getCurrentAndCorrectlyStop() {
    State* thirdState;
    State* secondState;
    State* firstState;
    thirdState = new State([&thirdState](){std::cerr << "thirdState\n";return thirdState;});
    secondState = new State([&thirdState](){std::cerr << "secondState\n";return thirdState;});
    firstState = new State([&secondState](){std::cerr << "firstState\n";return secondState;});

    State *states[] = {firstState, secondState, thirdState};
    StateMachine machine(3, states );
    machine.start();

    // Should initially be at the first state
    TEST_ASSERT_TRUE(machine.current(firstState));

    // then second state
    machine.handle();
    TEST_ASSERT_TRUE(machine.current(secondState));

    // then third state
    machine.handle();
    TEST_ASSERT_TRUE(machine.current(thirdState));

    // well advance and should always end in last state
    machine.handle();
    TEST_ASSERT_TRUE(machine.current(thirdState));
}

void should_testsTimedStateSingleRun() {
    State* thirdState;
    State* secondState;
    State* firstState;
    thirdState = new State([&thirdState](){std::cerr << "thirdState\n";return thirdState;});
    secondState = new StateTimed(10, [&thirdState](){std::cerr << "secondState\n";return thirdState;});
    firstState = new State([&secondState](){std::cerr << "firstState\n";return secondState;});

    State *states[] = {firstState, secondState, thirdState};
    StateMachine machine(3, states );
    machine.start();
    // Should initially be at the first state
    TEST_ASSERT_TRUE(machine.current(firstState));

    // then second state
    machine.handle();
    TEST_ASSERT_TRUE(machine.current(secondState));

    // Still at second state
    machine.handle();
    TEST_ASSERT_TRUE(machine.current(secondState));

    // should have been advanced to third state
    millisStubbed = 11;
    machine.handle();
    TEST_ASSERT_TRUE(machine.current(thirdState));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(should_getCurrentAndCorrectlyStop);
    RUN_TEST(should_getCurrentAndCorrectlyStop);
    RUN_TEST(should_testsTimedStateSingleRun);
    UNITY_END();

    return 0;
}