//
// Created by wlanjie on 2018/2/7.
//

#ifndef MP4_STRING_H
#define MP4_STRING_H

#include "types.h"

namespace mp4 {
// TODO name
class String {
public:
    // constructors
    String();
    String(const char* s);
    String(const char* s, Size size);
    String(const String& s);
    explicit String(Size size);

    // destructor
    ~String();

    // operators
    const String& operator=(const String& s);
    const String& operator=(const char* s);
    char operator[](unsigned int index) const {
        return m_Chars[index];
    }
    bool operator==(const String& s) const;
    bool operator!=(const String& s) const { return !(*this == s); }
    bool operator==(const char* s) const;
    bool operator!=(const char* s) const { return !(*this == s); }

    // methods
    Size getLength() const { return m_Length; }
    const char* getChars() const { return m_Chars;  }
    char* useChars() { return m_Chars;  }
    void assign(const char *chars, Size size);
    int find(char c, unsigned int start = 0) const;

private:
    // class members
    static char EmptyString;

    // members
    char* m_Chars;
    Size m_Length;
};

}
#endif //MP4_STRING_H
