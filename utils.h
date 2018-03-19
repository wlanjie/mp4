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

/*----------------------------------------------------------------------
|   global options
+---------------------------------------------------------------------*/
class GlobalOptions
{
public:
    static bool        GetBool(const char* name);
    static void        SetBool(const char* name, bool value);
    static const char* GetString(const char* name);
    static void        SetString(const char* name, const char* value);

private:
    struct Entry {
        String m_Name;
        String m_Value;
    };
    static Entry* GetEntry(const char* name, bool autocreate);
    static List<Entry>* g_Entries;
};

/*----------------------------------------------------------------------
|   non-inline functions
+---------------------------------------------------------------------*/
double   BytesToDoubleBE(const unsigned char* bytes);
UI64 BytesToUInt64BE(const unsigned char* bytes);
void BytesFromDoubleBE(unsigned char* bytes, double value);
void BytesFromUInt64BE(unsigned char* bytes, UI64 value);

/*----------------------------------------------------------------------
|   BytesToUInt32BE
+---------------------------------------------------------------------*/
inline UI32
BytesToUInt32BE(const unsigned char* bytes)
{
    return
            ( ((UI32)bytes[0])<<24 ) |
            ( ((UI32)bytes[1])<<16 ) |
            ( ((UI32)bytes[2])<<8  ) |
            ( ((UI32)bytes[3])     );
}

/*----------------------------------------------------------------------
|   BytesToInt32BE
+---------------------------------------------------------------------*/
inline SI32
BytesToInt32BE(const unsigned char* bytes)
{
    return BytesToUInt32BE(bytes);
}

/*----------------------------------------------------------------------
|   BytesToUInt24BE
+---------------------------------------------------------------------*/
inline UI32
BytesToUInt24BE(const unsigned char* bytes)
{
    return
            ( ((UI32)bytes[0])<<16 ) |
            ( ((UI32)bytes[1])<<8  ) |
            ( ((UI32)bytes[2])     );
}

/*----------------------------------------------------------------------
|   BytesToInt16BE
+---------------------------------------------------------------------*/
inline UI16
BytesToUInt16BE(const unsigned char* bytes)
{
    return
            ( ((UI16)bytes[0])<<8  ) |
            ( ((UI16)bytes[1])     );
}

/*----------------------------------------------------------------------
|   BytesToInt16BE
+---------------------------------------------------------------------*/
inline SI16
BytesToInt16BE(const unsigned char* bytes)
{
    return (SI16)BytesToUInt16BE(bytes);
}

/*----------------------------------------------------------------------
|   BytesFromUInt32BE
+---------------------------------------------------------------------*/
inline void
BytesFromUInt32BE(unsigned char* bytes, UI32 value)
{
    bytes[0] = (unsigned char)((value >> 24)&0xFF);
    bytes[1] = (unsigned char)((value >> 16)&0xFF);
    bytes[2] = (unsigned char)((value >>  8)&0xFF);
    bytes[3] = (unsigned char)((value      )&0xFF);
}

/*----------------------------------------------------------------------
|   BytesFromUInt24BE
+---------------------------------------------------------------------*/
inline void
BytesFromUInt24BE(unsigned char* bytes, UI32 value)
{
    bytes[0] = (unsigned char)((value >> 16)&0xFF);
    bytes[1] = (unsigned char)((value >>  8)&0xFF);
    bytes[2] = (unsigned char)((value      )&0xFF);
}

/*----------------------------------------------------------------------
|   BytesFromUInt16BE
+---------------------------------------------------------------------*/
inline void
BytesFromUInt16BE(unsigned char* bytes, UI16 value)
{
    bytes[0] = (unsigned char)((value >> 8)&0xFF);
    bytes[1] = (unsigned char)((value     )&0xFF);
}

/*----------------------------------------------------------------------
|   time functions
+---------------------------------------------------------------------*/
UI32 DurationMsFromUnits(UI64 units,
                                 UI32 units_per_second);
UI64 ConvertTime(UI64 time_value,
                         UI32 from_time_scale,
                         UI32 to_time_scale);

/*----------------------------------------------------------------------
|   random numbers
+---------------------------------------------------------------------*/
Result
System_GenerateRandomBytes(UI08* buffer, Size buffer_size);

/*----------------------------------------------------------------------
|   string utils
+---------------------------------------------------------------------*/
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

unsigned char HexNibble(char c);
char NibbleHex(unsigned int nibble);
void FormatFourChars(char* str, UI32 value);
void FormatFourCharsPrintable(char* str, UI32 value);
Result
ParseHex(const char* hex, unsigned char* bytes, unsigned int count);
Result
FormatHex(const UI08* data, unsigned int data_size, char* hex);
Result
SplitArgs(char* arg, char*& arg0, char*& arg1, char*& arg2);
Result
SplitArgs(char* arg, char*& arg0, char*& arg1);
UI32
ParseIntegerU(const char* value);

/*----------------------------------------------------------------------
|   BitWriter
+---------------------------------------------------------------------*/
class BitWriter
{
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

    void Write(UI32 bits, unsigned int bit_count);

    unsigned int GetBitCount()     { return m_BitCount; }
    const unsigned char* GetData() { return m_Data;     }

private:
    unsigned char* m_Data;
    unsigned int   m_DataSize;
    unsigned int   m_BitCount;
};

/*----------------------------------------------------------------------
|   BitReader
+---------------------------------------------------------------------*/
class BitReader
{
public:
    // types
    typedef unsigned int BitsWord;

    // constructor and destructor
    BitReader(const UI08* data, unsigned int data_size);
    ~BitReader();

    // methods
    Result   Reset();
    int          ReadBit();
    UI32     ReadBits(unsigned int bit_count);
    int          PeekBit();
    UI32     PeekBits(unsigned int bit_count);
    Result   SkipBytes(Size byte_count);
    void         SkipBit();
    void         SkipBits(unsigned int bit_count);

    unsigned int GetBitsRead();

private:
    // methods
    BitsWord ReadCache() const;

    // members
    DataBuffer m_Buffer;
    unsigned int   m_Position;
    BitsWord       m_Cache;
    unsigned int   m_BitsCached;
};


#endif //MP4_UTILS_H
