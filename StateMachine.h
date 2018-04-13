#pragma once
#include <stdint.h>

extern long unsigned int millis();

template<class T> class StateMachine {
private:
    const uint8_t m_arraySize;
    T* m_waiters;
    uint32_t* m_timeToWait;
    T m_lastItem;

    bool* m_isOrWasWaiting;
    T m_status;
    uint32_t m_startTime;
    uint32_t m_waitTime;
    bool m_bootSequenceFinnished;

    int8_t isWaiter() const {
        int8_t i = m_arraySize;

        while (--i >= 0) {
            if (m_status == m_waiters[i]) {
                return i;
            }
        }

        return -1;
    }
public:
    StateMachine(const uint8_t p_size,
                 T* p_waiters,
                 uint32_t* p_timeToWait,
                 T p_lastItem) :
        m_arraySize(p_size),
        m_waiters(p_waiters),
        m_timeToWait(p_timeToWait),
        m_lastItem(p_lastItem),
        m_status(static_cast<T>(0)),
        m_startTime(0),
        m_waitTime(0),
        m_bootSequenceFinnished(false) {
        m_isOrWasWaiting = new bool[p_size];

        for (uint8_t i = 0; i < m_arraySize; i++) {
            m_isOrWasWaiting[i] = false;
        }
    }

    virtual ~StateMachine() {
        delete m_isOrWasWaiting;
    }

    void handle() {
        if (m_waitTime > 0) {
            if (millis() - m_startTime > m_waitTime) {
                m_startTime = 0;
                m_waitTime = 0;
                advance();
            }
        }
    }

    T current() const {
        return m_status;
    }

    void moveTo(const T p_status) {
        m_status = p_status;
        m_startTime = 0;
        m_waitTime = 0;
    }

    bool finnished() const {
        return m_bootSequenceFinnished;
    }

    void advance() {
        if (m_status == m_lastItem) {
            return;
        }

        m_status = static_cast<T>(static_cast<uint8_t>(m_status) + 1);

        if (m_status == m_lastItem) {
            m_bootSequenceFinnished = true;
            return;
        }

        const int8_t waiterLoc = isWaiter();

        if (waiterLoc >= 0) {
            // Setup a waiting time for this item, when time is up we advance automatically
            if (m_isOrWasWaiting[waiterLoc] == false) {
                m_isOrWasWaiting[waiterLoc] = true;
                m_waitTime = m_timeToWait[waiterLoc];
                m_startTime = millis();
            } else {
                advance();
            }
        }
    }
};
