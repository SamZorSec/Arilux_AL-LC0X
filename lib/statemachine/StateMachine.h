#pragma once
#include <stdint.h>
#include <functional>

/**
 * Simpel state that gets run each time the StateMachine reaches this state
 */
class State {
    friend class StateMachine;
public:
    typedef std::function<const State* (void)> TValueFunction;

private:
    static uint16_t m_oCounter;
    uint16_t oCounter() {
        return m_oCounter++;
    }
    const uint16_t m_id;
    const TValueFunction m_run;

protected:
    uint16_t id() const;

    virtual void transitionStart(const uint32_t p_currentTime) const {}
    virtual void transitionEnd(const uint32_t p_currentTime) const {}
    virtual const State* run(const uint32_t currentMillis) const;

public:
    State(const TValueFunction p_run);  

    inline bool operator==(const State& other) const { 
            return other.m_id == m_id; 
    }
    inline bool operator!=(const State& other) const { 
            return !(*this == other); 
    }
};

/**
 * State that will run after given time has passed
 */
class StateTimed : public State {
    const uint32_t m_forTime;
    mutable uint32_t m_startTime;

public:
    StateTimed(const uint32_t p_forTime, const TValueFunction p_run);

private:
    virtual void transitionStart(const uint32_t p_currentTime) const;
    virtual const State* run(const uint32_t p_currentTime) const;
};


/**
 * StateMachine itself that will run through all states
 */
class StateMachine {
private:
    size_t m_pos;
    const size_t m_size;
    // TODO: Multi dimentional array here for efficienty???
    State* *m_states; 
public:
    StateMachine(size_t p_size, State* *p_states);

    // Call start eonce after you created the state machine but as aclose as possible
    // to your loop function
    void start() const;

    // Evaluates if the current state is the given state
    bool current(const State *state) const;

    // Call in your loop function regularly
    void handle();
};

