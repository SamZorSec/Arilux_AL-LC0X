
#include "optparser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

OptValue::OptValue(const uint8_t p_pos, const char* p_key, const char* p_value) :
    m_pos(p_pos), m_key(p_key), m_value(p_value) {
}

const char* OptValue::key() const {
    return m_key;
}

int16_t OptValue::asInt() const {
    return atoi(m_value);
}

int32_t OptValue::asLong() const {
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


void OptParser::get(const char* p_options, const char *m_sep, TValueFunction callBack) {
    char* token;
    char* work = strdup(p_options);
    work = trimwhitespace(work);
    cleanUp(false, m_sep[0], work, strlen(work), 0);
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

    free(work);
}

bool OptParser::cleanUp(bool didCleanup, const char m_sep, char* str,  size_t length, size_t pos) {
    if (pos >= length) {
        return didCleanup;
    }

    if (
        (str[pos] == '=' && str[pos + 1] == ' ') ||
        (str[pos] == ' ' && str[pos + 1] == '=')
       ) {
        char* p;
        char* s;

        if (str[pos + 1] == '=' || str[pos + 1] == m_sep) {
            p = &str[pos + 0];
            s = &str[pos + 1];
        } else {
            p = &str[pos + 1];
            s = &str[pos + 2];
        }

        while (((*p++) = (*s++)) != '\0');

        return cleanUp(true, m_sep, str, strlen(str), pos);
    } else {
        return cleanUp(didCleanup, m_sep, str, length, pos + 1);
    }
}

bool OptParser::isNext(char* str, char find) {
    size_t length = strlen(str);
    size_t pos = 0;
    while (str[pos++] == ' ' && pos < length);
    return str[pos] == find;
}

char *OptParser::trimwhitespace(char *str)
{
    if (str == nullptr) {
        return nullptr;
    }
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}
