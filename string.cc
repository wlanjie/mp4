//
// Created by wlanjie on 2018/2/7.
//

#include "string.h"
#include "utils.h"

namespace mp4 {
// TODO name
char String::EmptyString = 0;

String::String() : m_Chars(&EmptyString), m_Length(0) {}

String::String(const char* s) {
    if (s == NULL) {
        m_Chars = &EmptyString;
        m_Length = 0;
        return;
    }
    m_Length = (Size)StringLength(s);
    m_Chars = new char[m_Length+1];
    CopyMemory(m_Chars, s, m_Length+1);
}

String::String(const char* s, Size size) :
        m_Chars(new char[size+1]),
        m_Length(size)
{
    m_Chars[size] = 0;
    CopyMemory(m_Chars, s, size);
}

String::String(const String& s) {
    m_Length = s.m_Length;
    m_Chars = new char[m_Length+1];
    CopyMemory(m_Chars, s.m_Chars, m_Length+1);
}

String::String(Size size) {
    m_Length = size;
    m_Chars = new char[m_Length+1];
    for (unsigned int i=0; i<size+1; i++) m_Chars[i] = 0;
}

String::~String() {
    if (m_Chars != &EmptyString) delete[] m_Chars;
}

const String& String::operator=(const String& s) {
    if (&s == this) return s;
    if (m_Chars != &EmptyString) delete[] m_Chars;
    m_Length = s.m_Length;
    m_Chars = new char[m_Length+1];
    CopyMemory(m_Chars, s.m_Chars, m_Length+1);

    return *this;
}

const String& String::operator=(const char* s) {
    if (s == NULL) {
        if (m_Chars != &EmptyString) delete[] m_Chars;
        m_Chars = &EmptyString;
        m_Length = 0;
    } else {
        Assign(s, (Size)StringLength(s));
    }

    return *this;
}

bool String::operator==(const String& s) const {
    if (m_Length != s.m_Length) return false;
    for (unsigned int i=0; i<m_Length; i++) {
        if (m_Chars[i] != s.m_Chars[i]) return false;
    }
    return true;
}

bool String::operator==(const char* s) const {
    Size s_length = (Size)StringLength(s);
    if (m_Length != s_length) return false;
    for (unsigned int i=0; i<s_length; i++) {
        if (m_Chars[i] != s[i]) return false;
    }
    return true;
}

void String::Assign(const char* s, Size size) {
    if (m_Chars != &EmptyString) delete[] m_Chars;
    m_Length = size;
    m_Chars = new char[m_Length+1];
    CopyMemory(m_Chars, s, m_Length);
    m_Chars[size] = '\0';
}

int String::Find(char c, unsigned int start) const {
    const char* chars = GetChars();
    for (unsigned int i=start; i<m_Length; i++) {
        if (chars[i] == c) return i;
    }
    return -1;
}

}
