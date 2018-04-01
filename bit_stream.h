//
// Created by wlanjie on 2018/3/15.
//
// TODO
#ifndef _BIT_STREAM_H_
#define _BIT_STREAM_H_

#include "types.h"
#include "results.h"

namespace mp4 {

const int ERROR_BASE_BITSTREAM = -10000;

// the max frame size we can handle
const unsigned int BITSTREAM_BUFFER_SIZE = 8192;

// flags
#define BITSTREAM_FLAG_EOS 0x01

// error codes
const int ERROR_CORRUPTED_BITSTREAM = (ERROR_BASE_BITSTREAM - 0);
const int ERROR_NOT_ENOUGH_FREE_BUFFER = (ERROR_BASE_BITSTREAM - 1);

/* use long by default */
typedef unsigned int BitsWord;
#define WORD_BITS  32
#define WORD_BYTES 4

class BitStream {
public:
    // constructor and destructor
    BitStream();

    ~BitStream();

    // methods
    Result reset();

    Size getContiguousBytesFree();

    Size getBytesFree();

    Result writeBytes(const UI08 *bytes, Size byte_count);

    Size getContiguousBytesAvailable();

    Size getBytesAvailable();

    UI08 readByte();

    Result readBytes(UI08 *bytes, Size byte_count);

    UI08 peekByte();

    Result peekBytes(UI08 *bytes, Size byte_count);

    int readBit();

    UI32 readBits(unsigned int bit_count);

    int peekBit();

    UI32 peekBits(unsigned int bit_count);

    Result skipBytes(Size byte_count);

    void skipBit();

    void skipBits(unsigned int bit_count);

    Result byteAlign();

    // members
    UI08 *m_Buffer;
    unsigned int m_In;
    unsigned int m_Out;
    BitsWord m_Cache;
    unsigned int m_BitsCached;
    unsigned int m_Flags;

private:
    // methods
    BitsWord readCache() const;
};

#define BIT_MASK(_n) ((1<<(_n))-1)

#define BITSTREAM_POINTER_VAL(offset) \
    ((offset)&(BITSTREAM_BUFFER_SIZE-1))

#define BITSTREAM_POINTER_OFFSET(pointer, offset) \
    (BITSTREAM_POINTER_VAL((pointer)+(offset)))

#define BITSTREAM_POINTER_ADD(pointer, offset) \
    ((pointer) = BITSTREAM_POINTER_OFFSET(pointer, offset))

inline BitsWord BitStream::readCache() const {
    unsigned int pos = m_Out;
    BitsWord cache;

#if WORD_BITS != 32
#error unsupported word size /* 64 and other word size not yet implemented */
#endif

    if (pos <= BITSTREAM_BUFFER_SIZE - WORD_BYTES) {
        unsigned char *out_ptr = &m_Buffer[pos];
        cache = (((BitsWord) out_ptr[0]) << 24)
                | (((BitsWord) out_ptr[1]) << 16)
                | (((BitsWord) out_ptr[2]) << 8)
                | (((BitsWord) out_ptr[3]));
    } else {
        unsigned char *buf_ptr = m_Buffer;
        cache = (((BitsWord) buf_ptr[pos]) << 24)
                | (((BitsWord) buf_ptr[BITSTREAM_POINTER_OFFSET (pos, 1)]) << 16)
                | (((BitsWord) buf_ptr[BITSTREAM_POINTER_OFFSET (pos, 2)]) << 8)
                | (((BitsWord) buf_ptr[BITSTREAM_POINTER_OFFSET (pos, 3)]));
    }

    return cache;
}

inline UI32 BitStream::readBits(unsigned int n) {
    BitsWord result;
    if (m_BitsCached >= n) {
        /* we have enough bits in the cache to satisfy the request */
        m_BitsCached -= n;
        result = (m_Cache >> m_BitsCached) & BIT_MASK(n);
    } else {
        /* not enough bits in the cache */
        BitsWord word;

        /* read the next word */
        {
            word = readCache();
            m_Out = BITSTREAM_POINTER_OFFSET(m_Out, WORD_BYTES);
        }

        /* combine the new word and the cache, and update the state */
        {
            BitsWord cache = m_Cache & BIT_MASK(m_BitsCached);
            n -= m_BitsCached;
            m_BitsCached = WORD_BITS - n;
            result = (word >> m_BitsCached) | (cache << n);
            m_Cache = word;
        }
    }

    return result;
}

inline int BitStream::readBit() {
    BitsWord result;
    if (m_BitsCached == 0) {
        /* the cache is empty */

        /* read the next word into the cache */
        m_Cache = readCache();
        m_Out = BITSTREAM_POINTER_OFFSET(m_Out, WORD_BYTES);
        m_BitsCached = WORD_BITS - 1;

        /* return the first bit */
        result = m_Cache >> (WORD_BITS - 1);
    } else {
        /* get the bit from the cache */
        result = (m_Cache >> (--m_BitsCached)) & 1;
    }
    return result;
}

inline UI32 BitStream::peekBits(unsigned int n) {
    /* we have enough bits in the cache to satisfy the request */
    if (m_BitsCached >= n) {
        return (m_Cache >> (m_BitsCached - n)) & BIT_MASK(n);
    } else {
        /* not enough bits in the cache, read the next word */
        BitsWord word = readCache();

        /* combine the new word and the cache, and update the state */
        BitsWord cache = m_Cache & BIT_MASK(m_BitsCached);
        n -= m_BitsCached;
        return (word >> (WORD_BITS - n)) | (cache << n);
    }
}

inline int BitStream::peekBit() {
    /* the cache is empty */
    if (m_BitsCached == 0) {
        /* read the next word into the cache */
        BitsWord cache = readCache();

        /* return the first bit */
        return cache >> (WORD_BITS - 1);
    } else {
        /* get the bit from the cache */
        return (m_Cache >> (m_BitsCached - 1)) & 1;
    }
}

inline void BitStream::skipBits(unsigned int n) {
    if (n <= m_BitsCached) {
        m_BitsCached -= n;
    } else {
        n -= m_BitsCached;
        while (n >= WORD_BITS) {
            m_Out = BITSTREAM_POINTER_OFFSET(m_Out, WORD_BYTES);
            n -= WORD_BITS;
        }
        if (n) {
            m_Cache = readCache();
            m_BitsCached = WORD_BITS - n;
            m_Out = BITSTREAM_POINTER_OFFSET(m_Out, WORD_BYTES);
        } else {
            m_BitsCached = 0;
            m_Cache = 0;
        }
    }
}

inline void BitStream::skipBit() {
    if (m_BitsCached == 0) {
        m_Cache = readCache();
        m_Out = BITSTREAM_POINTER_OFFSET(m_Out, WORD_BYTES);
        m_BitsCached = WORD_BITS - 1;
    } else {
        --m_BitsCached;
    }
}

inline UI08 BitStream::readByte() {
    skipBits(m_BitsCached & 7);
    return (UI08) readBits(8);
}

inline UI08 BitStream::peekByte() {
    unsigned int extra_bits = m_BitsCached & 7;
    return (UI08) (peekBits(extra_bits + 8) & 0xFF);
}

}
#endif // _BIT_STREAM_H_
