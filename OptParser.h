#pragma once

#include <stdint.h>
#include <functional>

class OptValue {
private:
    const uint8_t m_pos;
    const char* m_key;
    const char* m_value;
public:
    OptValue(const uint8_t m_pos, const char* p_key, const char* p_value);
    const char* key() const;
    uint16_t asInt() const;
    uint32_t asLong() const;
    float asFloat() const;
    const char* asChar() const;
    uint8_t pos() const;
};


class OptParser {
public:
    typedef std::function<void(const OptValue&)> TValueFunction;
    static void get(const char* p_options, TValueFunction f);
    static void get(const char* p_options, const char* sep, TValueFunction f);
private:
    // Very simply cleanup routine to remove some whitespace from the string
    // while respecting space between variables
    static void cleanUp(char* str, uint8_t pos);
};

