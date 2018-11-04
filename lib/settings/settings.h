#pragma once

#include <stdint.h>
#include <functional>

class Settings final {
public:
    typedef std::function<const void (void)> SaveFunction;
    typedef std::function<const bool (void)> ModifiedFunction;
private:
    const uint32_t m_debounceWaitTime;
    const uint32_t m_minWaitCommitTime; 
    uint32_t m_startCommitTime;
    uint32_t m_startDebounceTime;
    bool m_modifiedLatch;
    const SaveFunction m_callSave;
    const ModifiedFunction m_callModified;
public:
    Settings(const uint32_t p_debounceWaitTime, // Time to wait to holdoff commits
             const uint32_t p_minWaitCommitTime, // Minimum time to wait between commits
             const SaveFunction p_save,
             const ModifiedFunction p_modified);
    void handle();
    void save();
    void save(bool force);
};


