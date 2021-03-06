//
// Created by wlanjie on 2018/2/7.
//

#include "utils.h"

namespace mp4 {
  // TODO name

List<GlobalOptions::Entry> *GlobalOptions::g_Entries = nullptr;

GlobalOptions::Entry* GlobalOptions::getEntry(const char *name, bool autocreate) {
    if (g_Entries == nullptr) {
        g_Entries = new List<Entry>;
    }
    for (List<Entry>::Item *item = g_Entries->firstItem();
         item;
         item = item->getNext()) {
        if (item->getData()->m_Name == name) return item->getData();
    }

    if (autocreate) {
        auto *new_entry = new Entry();
        new_entry->m_Name = name;
        g_Entries->add(new_entry);
        return new_entry;
    } else {
        return nullptr;
    }
}

void GlobalOptions::setBool(const char *name, bool value) {
    Entry *entry = getEntry(name, true);
    entry->m_Value = value ? "true" : "false";
}

bool GlobalOptions::getBool(const char *name) {
    Entry *entry = getEntry(name, false);
    if (entry) {
        return entry->m_Value == "true";
    } else {
        return false;
    }
}

void GlobalOptions::setString(const char *name, const char *value) {
    Entry *entry = getEntry(name, true);
    entry->m_Value = value;
}

const char * GlobalOptions::getString(const char *name) {
    Entry *entry = getEntry(name, false);
    if (entry) {
        return entry->m_Value.getChars();
    } else {
        return nullptr;
    }
}

double bytesToDoubleBE(const unsigned char *bytes) {
    UI64 i_value = bytesToUInt64BE(bytes);
    auto *v_value = reinterpret_cast<void *>(&i_value);
    auto *d_value = reinterpret_cast<double *>(v_value);

    return *d_value;
}

UI64 bytesToUInt64BE(const unsigned char *bytes) {
    return (((UI64) bytes[0]) << 56) |
           (((UI64) bytes[1]) << 48) |
           (((UI64) bytes[2]) << 40) |
           (((UI64) bytes[3]) << 32) |
           (((UI64) bytes[4]) << 24) |
           (((UI64) bytes[5]) << 16) |
           (((UI64) bytes[6]) << 8) |
           (((UI64) bytes[7]));
}

void bytesFromDoubleBE(unsigned char *bytes, double value) {
    auto *v_value = reinterpret_cast<void *>(&value);
    auto *i_value = reinterpret_cast<UI64 *>(v_value);

    bytesFromUInt64BE(bytes, *i_value);
}

void bytesFromUInt64BE(unsigned char *bytes, UI64 value) {
    bytes[0] = (unsigned char) ((value >> 56) & 0xFF);
    bytes[1] = (unsigned char) ((value >> 48) & 0xFF);
    bytes[2] = (unsigned char) ((value >> 40) & 0xFF);
    bytes[3] = (unsigned char) ((value >> 32) & 0xFF);
    bytes[4] = (unsigned char) ((value >> 24) & 0xFF);
    bytes[5] = (unsigned char) ((value >> 16) & 0xFF);
    bytes[6] = (unsigned char) ((value >> 8) & 0xFF);
    bytes[7] = (unsigned char) ((value) & 0xFF);
}

UI32 durationMsFromUnits(UI64 units, UI32 unitsPerSecond) {
    if (unitsPerSecond == 0) return 0;
    return (UI32) (((double) units * 1000.0) / (double) unitsPerSecond);
}

UI64 convertTime(UI64 timeValue, UI32 fromTimeScale, UI32 toTimeScale) {
    if (fromTimeScale == 0) return 0;
    double ratio = (double) toTimeScale / (double) fromTimeScale;
    return ((UI64) (0.5 + (double) timeValue * ratio));
}

void formatFourChars(char *str, UI32 value) {
    str[0] = (value >> 24) & 0xFF;
    str[1] = (value >> 16) & 0xFF;
    str[2] = (value >> 8) & 0xFF;
    str[3] = (value) & 0xFF;
    str[4] = '\0';
}

void formatFourCharsPrintable(char *str, UI32 value) {
    formatFourChars(str, value);
    for (int i = 0; i < 4; i++) {
        if (str[i] < ' ' || str[i] >= 127) {
            str[i] = '.';
        }
    }
}

Result splitArgs(char *arg, char *&arg0, char *&arg1) {
    arg0 = arg;
    char *c = arg;
    while (*c != 0 && *c != ':') {
        c++;
    }
    if (*c == ':') {
        *c++ = '\0';
        arg1 = c;
        return SUCCESS;
    } else {
        return FAILURE;
    }
}

Result splitArgs(char *arg, char *&arg0, char *&arg1, char *&arg2) {
    Result result = splitArgs(arg, arg0, arg1);
    if (FAILED(result)) return result;
    return splitArgs(arg1, arg1, arg2);
}

unsigned char hexNibble(char c) {
    switch (c) {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        case 'a':
        case 'A':
            return 10;
        case 'b':
        case 'B':
            return 11;
        case 'c':
        case 'C':
            return 12;
        case 'd':
        case 'D':
            return 13;
        case 'e':
        case 'E':
            return 14;
        case 'f':
        case 'F':
            return 15;
        default:
            return 0;
    }
}

char nibbleHex(unsigned int nibble) {
    if (nibble < 10) {
        return (char) ('0' + nibble);
    } else if (nibble < 16) {
        return (char) ('A' + (nibble - 10));
    } else {
        return ' ';
    }
}

Result parseHex(const char *hex, unsigned char *bytes, unsigned int count) {
    if (StringLength(hex) < 2 * count) return ERROR_INVALID_PARAMETERS;
    for (unsigned int i = 0; i < count; i++) {
        bytes[i] = (hexNibble(hex[2 * i]) << 4) | (hexNibble(hex[2 * i + 1]));
    }
    return SUCCESS;
}

Result formatHex(const UI08 *data, unsigned int data_size, char *hex) {
    for (unsigned int i = 0; i < data_size; i++) {
        *hex++ = nibbleHex(data[i] >> 4);
        *hex++ = nibbleHex(data[i] & 0x0F);
    }

    return SUCCESS;
}

void BitWriter::write(UI32 bits, unsigned int bit_count) {
    unsigned char *data = m_Data;
    if (m_BitCount + bit_count > m_DataSize * 8) return;
    data += m_BitCount / 8;
    unsigned int space = 8 - (m_BitCount % 8);
    while (bit_count) {
        unsigned int mask = bit_count == 32 ? 0xFFFFFFFF : ((1 << bit_count) - 1);
        if (bit_count <= space) {
            *data |= ((bits & mask) << (space - bit_count));
            m_BitCount += bit_count;
            return;
        } else {
            *data |= ((bits & mask) >> (bit_count - space));
            ++data;
            m_BitCount += space;
            bit_count -= space;
            space = 8;
        }
    }
}

UI32 parseIntegerU(const char *str) {
    if (str == NULL) {
        return 0;
    }

    // parse the digits
    UI32 value = 0;
    while (char c = *str++) {
        if (c >= '0' && c <= '9') {
            value = 10 * value + (c - '0');
        } else {
            return 0;
        }
    }

    return value;
}

#define WORD_BITS  32
#define WORD_BYTES 4

#define BIT_MASK(_n) ((1<<(_n))-1)

#if WORD_BITS != 32
#error unsupported word size /* 64 and other word size not yet implemented */
#endif

BitReader::BitReader(const UI08 *data, unsigned int data_size) :
        m_Position(0),
        m_Cache(0),
        m_BitsCached(0) {
    // make the buffer an integral mulitple of the word size
    m_Buffer.setBufferSize(WORD_BYTES * ((data_size + WORD_BYTES - 1) / WORD_BYTES));
    m_Buffer.setData(data, data_size);
    if (m_Buffer.getDataSize() != m_Buffer.getBufferSize()) {
        SetMemory(m_Buffer.useData() + m_Buffer.getDataSize(), 0, m_Buffer.getBufferSize() - m_Buffer.getDataSize());
    }
}

BitReader::~BitReader() {
}

Result BitReader::reset() {
    m_Position = 0;
    m_Cache = 0;
    m_BitsCached = 0;

    return SUCCESS;
}

unsigned int BitReader::getBitsRead() {
    return 8 * m_Position - m_BitsCached;
}

BitReader::BitsWord BitReader::readCache() const {
    const UI08 *out_ptr = m_Buffer.getData() + m_Position;
    return (((BitReader::BitsWord) out_ptr[0]) << 24) |
           (((BitReader::BitsWord) out_ptr[1]) << 16) |
           (((BitReader::BitsWord) out_ptr[2]) << 8) |
           (((BitReader::BitsWord) out_ptr[3]));
}

UI32 BitReader::readBits(unsigned int n) {
    if (n == 0) return 0;
    BitReader::BitsWord result;
    if (m_BitsCached >= n) {
        /* we have enough bits in the cache to satisfy the request */
        m_BitsCached -= n;
        result = (m_Cache >> m_BitsCached) & BIT_MASK(n);
    } else {
        /* not enough bits in the cache */
        BitReader::BitsWord word = readCache();
        m_Position += WORD_BYTES;

        /* combine the new word and the cache, and update the state */
        BitReader::BitsWord cache = m_Cache & BIT_MASK(m_BitsCached);
        n -= m_BitsCached;
        m_BitsCached = WORD_BITS - n;
        result = m_BitsCached ? (word >> m_BitsCached) | (cache << n) : word;
        m_Cache = word;
    }

    return result;
}

int BitReader::readBit() {
    BitReader::BitsWord result;
    if (m_BitsCached == 0) {
        /* the cache is empty */

        /* read the next word into the cache */
        m_Cache = readCache();
        m_Position += WORD_BYTES;
        m_BitsCached = WORD_BITS - 1;

        /* return the first bit */
        result = m_Cache >> (WORD_BITS - 1);
    } else {
        /* get the bit from the cache */
        result = (m_Cache >> (--m_BitsCached)) & 1;
    }
    return result;
}

UI32 BitReader::peekBits(unsigned int n) {
    /* we have enough bits in the cache to satisfy the request */
    if (m_BitsCached >= n) {
        return (m_Cache >> (m_BitsCached - n)) & BIT_MASK(n);
    } else {
        /* not enough bits in the cache, read the next word */
        BitReader::BitsWord word = readCache();

        /* combine the new word and the cache, and update the state */
        BitReader::BitsWord cache = m_Cache & BIT_MASK(m_BitsCached);
        n -= m_BitsCached;
        return (word >> (WORD_BITS - n)) | (cache << n);
    }
}

int BitReader::peekBit() {
    /* the cache is empty */
    if (m_BitsCached == 0) {
        /* read the next word into the cache */
        BitReader::BitsWord cache = readCache();

        /* return the first bit */
        return cache >> (WORD_BITS - 1);
    } else {
        /* get the bit from the cache */
        return (m_Cache >> (m_BitsCached - 1)) & 1;
    }
}

void BitReader::skipBits(unsigned int n) {
    if (n <= m_BitsCached) {
        m_BitsCached -= n;
    } else {
        n -= m_BitsCached;
        while (n >= WORD_BITS) {
            m_Position += WORD_BYTES;
            n -= WORD_BITS;
        }
        if (n) {
            m_Cache = readCache();
            m_BitsCached = WORD_BITS - n;
            m_Position += WORD_BYTES;
        } else {
            m_BitsCached = 0;
            m_Cache = 0;
        }
    }
}

void BitReader::skipBit() {
    if (m_BitsCached == 0) {
        m_Cache = readCache();
        m_Position += WORD_BYTES;
        m_BitsCached = WORD_BITS - 1;
    } else {
        --m_BitsCached;
    }
}

}
