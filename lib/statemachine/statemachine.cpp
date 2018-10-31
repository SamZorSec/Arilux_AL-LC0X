#include "statemachine.h"

#ifndef UNIT_TEST
    #include <arduino.h>
#else
    extern "C" uint32_t millis();
#endif

uint16_t State::m_oCounter = 0;

State::State(TValueFunction p_run) :  
    m_id(oCounter()), 
    m_run(p_run) {      
}   

uint16_t State::id() const {
    return m_id;
}

const State* State::run(const uint32_t currentMillis) const {
    return m_run();   
}


StateTimed::StateTimed(uint32_t p_forTime, TValueFunction p_run) : 
    State(p_run), 
    m_forTime(p_forTime), 
    m_startTime(0) {
}

void StateTimed::transitionStart(const uint32_t p_currentTime) const {
    m_startTime = p_currentTime;
}

const State* StateTimed::run(const uint32_t p_currentTime) const {
    if (p_currentTime - m_startTime > m_forTime) {
        // We reset the time in case we re-run this state again
        m_startTime = p_currentTime;
        return State::run(p_currentTime);   
    }
    return this;
}


StateMachine::StateMachine(size_t p_size, State* *p_states) :
    m_pos(0), 
    m_size(p_size),
    m_states(p_states) {  
}

void StateMachine::start() const {
    m_states[0]->transitionStart(millis());
}

bool StateMachine::current(const State *state) const {
    return m_states[m_pos] == state;
}

void StateMachine::handle() {
    const uint32_t currentMillis = millis();
    const State* newState = m_states[m_pos]->run(currentMillis);
    // Test if we need to change state
    if (*m_states[m_pos] != *newState) {
        // If so, lookup it´s position in our state array
        for (uint16_t i=0;i<m_size;i++) {                
            if (*newState == *m_states[i]) {
                // When state isfound end the previous state and start the new state
                m_states[m_pos]->transitionEnd(currentMillis);
                m_pos = i;
                m_states[m_pos]->transitionStart(currentMillis);
                break;
            }
        }
    }
}

