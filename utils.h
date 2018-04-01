//
// Created by wlanjie on 2018/2/7.
//

#ifndef MP4_UTILS_H
#define MP4_UTILS_H

#include <cstdarg>
#include <cstdio>
#include "string.h"
#include "types.h"
#include "list.h"
#include "config.h"
#include "databuffer.h"

namespace mp4 {
  // TODO mp4

class GlobalOptions {
public:
    static bool        getBool(const char *name);
    static void        setBool(const char *name, bool value);
    static const char* getString(const char *name);
    static void        setString(const char *name, const char *value);

private:
    struct Entry {
        String m_Name;
        String m_Value;
    };
    static Entry* getEntry(const char *name, bool autocreate);
    static List<Entry>* g_Entries;
};

double bytesToDoubleBE(const unsigned char *bytes);
UI64 bytesToUInt64BE(const unsigned char *bytes);
void bytesFromDoubleBE(unsigned char *bytes, double value);
void bytesFromUInt64BE(unsigned char *bytes, UI64 value);

inline UI32 bytesToUInt32BE(const unsigned char *bytes) {
    return ( ((UI32)bytes[0])<<24 ) |
           ( ((UI32)bytes[1])<<16 ) |
           ( ((UI32)bytes[2])<<8  ) |
           ( ((UI32)bytes[3])     );
}

inline SI32 bytesToInt32BE(const unsigned char *bytes) {
    return bytesToUInt32BE(bytes);
}

inline UI32 bytesToUInt24BE(const unsigned char *bytes) {
    return ( ((UI32)bytes[0])<<16 ) |
           ( ((UI32)bytes[1])<<8  ) |
           ( ((UI32)bytes[2])     );
}

inline UI16 bytesToUInt16BE(const unsigned char *bytes) {
    return ( ((UI16)bytes[0])<<8  ) |
           ( ((UI16)bytes[1])     );
}

inline SI16 bytesToInt16BE(const unsigned char *bytes) {
    return (SI16) bytesToUInt16BE(bytes);
}

inline void bytesFromUInt32BE(unsigned char *bytes, UI32 value) {
    bytes[0] = (unsigned char)((value >> 24)&0xFF);
    bytes[1] = (unsigned char)((value >> 16)&0xFF);
    bytes[2] = (unsigned char)((value >>  8)&0xFF);
    bytes[3] = (unsigned char)((value      )&0xFF);
}

inline void bytesFromUInt24BE(unsigned char *bytes, UI32 value) {
    bytes[0] = (unsigned char)((value >> 16)&0xFF);
    bytes[1] = (unsigned char)((value >>  8)&0xFF);
    bytes[2] = (unsigned char)((value      )&0xFF);
}

inline void bytesFromUInt16BE(unsigned char *bytes, UI16 value) {
    bytes[0] = (unsigned char)((value >> 8)&0xFF);
    bytes[1] = (unsigned char)((value     )&0xFF);
}

UI32 durationMsFromUnits(UI64 units, UI32 unitsPerSecond);
UI64 convertTime(UI64 timeValue, UI32 fromTimeScale, UI32 toTimeScale);

Result System_GenerateRandomBytes(UI08* buffer, Size buffer_size);

#if defined (CONFIG_HAVE_STDIO_H)
#include <stdio.h>
#endif

#if defined (CONFIG_HAVE_SNPRINTF)
#define FormatString snprintf
#else
int FormatString(char* str, Size size, const char* format, ...);
#endif
#if defined(CONFIG_HAVE_VSNPRINTF)
#define FormatStringVN(s,c,f,a) vsnprintf(s,c,f,a)
#else
extern int FormatStringVN(char *buffer, size_t count, const char *format, va_list argptr);
#endif

#if defined (CONFIG_HAVE_STRING_H)
#include <string.h>
#define StringLength(x) strlen(x)
#define CopyMemory(x,y,z) memcpy(x,y,z)
#define CompareMemory(x, y, z) memcmp(x, y, z)
#define SetMemory(x,y,z) memset(x,y,z)
#define CompareStrings(x,y) strcmp(x,y)
#endif

unsigned char hexNibble(char c);
char nibbleHex(unsigned int nibble);
void formatFourChars(char *str, UI32 value);
void formatFourCharsPrintable(char *str, UI32 value);
Result parseHex(const char *hex, unsigned char *bytes, unsigned int count);
Result formatHex(const UI08 *data, unsigned int data_size, char *hex);
Result splitArgs(char *arg, char *&arg0, char *&arg1, char *&arg2);
Result splitArgs(char *arg, char *&arg0, char *&arg1);
UI32 parseIntegerU(const char *value);

class BitWriter {
public:
    BitWriter(Size size) : m_DataSize(size), m_BitCount(0) {
        if (size) {
            m_Data = new unsigned char[size];
            SetMemory(m_Data, 0, size);
        } else {
            m_Data = NULL;
        }
    }
    ~BitWriter() { delete[] m_Data; }

    void write(UI32 bits, unsigned int bit_count);

    unsigned int getBitCount()     { return m_BitCount; }
    const unsigned char* getData() { return m_Data;     }

private:
    unsigned char* m_Data;
    unsigned int   m_DataSize;
    unsigned int   m_BitCount;
};

class BitReader {
public:
    // types
    typedef unsigned int BitsWord;

    // constructor and destructor
    BitReader(const UI08* data, unsigned int data_size);
    ~BitReader();

    // methods
    Result reset();
    int readBit();
    UI32 readBits(unsigned int bit_count);
    int peekBit();
    UI32 peekBits(unsigned int bit_count);
    Result skipBytes(Size byte_count);
    void skipBit();
    void skipBits(unsigned int bit_count);

    unsigned int getBitsRead();

private:
    // methods
    BitsWord readCache() const;

    // members
    DataBuffer m_Buffer;
    unsigned int   m_Position;
    BitsWord       m_Cache;
    unsigned int   m_BitsCached;
};

}
#endif //MP4_UTILS_H
