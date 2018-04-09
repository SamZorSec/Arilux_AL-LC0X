
#include "OptParser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

OptValue::OptValue(const uint8_t p_pos, const char* p_key, const char* p_value) :
    m_pos(p_pos), m_key(p_key), m_value(p_value) {
}

const char* OptValue::key() const {
    return m_key;
}

uint16_t OptValue::asInt() const {
    return atoi(m_value);
}

uint32_t OptValue::asLong() const {
    return atol(m_value);
}

float OptValue::asFloat() const {
    return atof(m_value);
}

const char* OptValue::asChar() const {
    return m_value;
}

uint8_t OptValue::pos() const {
    return m_pos;
}


void OptParser::get(const char* p_options, TValueFunction f) {
    get(p_options, " ", f);
}

void OptParser::get(const char* p_options, const char* m_sep, TValueFunction callBack) {
    char* token;
    char* work = strdup(p_options);
    cleanUp(work, 0);
    uint8_t pos = 0;

    while ((token = strsep(&work, m_sep)) != NULL) {
        char* key;
        char* val;
        key = strsep(&token, "=");

        if (key != nullptr && strlen(key) > 0) {
            val = strsep(&token, "=");
            callBack(OptValue(pos++, key, val == nullptr ? key : val));
        }
    }

    delete(work);
}

void OptParser::cleanUp(char* str, uint8_t pos) {
    if (pos >= strlen(str)) {
        return;
    }

    /*
    if (str[pos] == '\'') {
        char* p=&str[pos+1];
        while( str[pos++]!= '\'');
    }*/

    if ((str[pos] == '=' && str[pos + 1] == ' ') ||
        (str[pos] == ' ' && str[pos + 1] == ' ') ||
        (str[pos] == ' ' && str[pos + 1] == '=')
       ) {
        char* p;
        char* s;

        if (str[pos + 1] == '=') {
            p = &str[pos + 0];
            s = &str[pos + 1];
        } else {
            p = &str[pos + 1];
            s = &str[pos + 2];
        }

        while ((*p++) = (*s++));

        cleanUp(str, pos);
    } else {
        cleanUp(str, pos + 1);
    }
}
