#include "bit_stream.h"
#include "utils.h"

namespace mp4 {

// TODO name
BitStream::BitStream() {
    m_Buffer = new UI08[BITSTREAM_BUFFER_SIZE];
    reset();
}

BitStream::~BitStream() {
    delete[] m_Buffer;
}

Result BitStream::reset() {
    m_In = 0;
    m_Out = 0;
    m_BitsCached = 0;
    m_Cache = 0;
    m_Flags = 0;

    return SUCCESS;
}

Result BitStream::byteAlign() {
    unsigned int to_flush = m_BitsCached & 7;
    if (to_flush > 0) skipBits(to_flush);

    return SUCCESS;
}

Size BitStream::getContiguousBytesFree() {
    return
            (m_In < m_Out) ?
            (m_Out - m_In - 1) :
            (m_Out == 0 ? (BITSTREAM_BUFFER_SIZE - m_In - 1) :
             (BITSTREAM_BUFFER_SIZE - m_In));
}

Size BitStream::getBytesFree() {
    return
            (m_In < m_Out) ?
            (m_Out - m_In - 1) :
            (BITSTREAM_BUFFER_SIZE + (m_Out - m_In) - 1);
}

Result BitStream::writeBytes(const UI08 *bytes,
                             Size byte_count) {
    /* check parameters */
    if (byte_count == 0) return SUCCESS;
    if (bytes == NULL) return ERROR_INVALID_PARAMETERS;

    /* check that we have enough space */
    if (getBytesFree() < byte_count) {
        return FAILURE;
    }

    /* write the bytes */
    if (m_In < m_Out) {
        CopyMemory(m_Buffer + m_In, bytes, byte_count);
        BITSTREAM_POINTER_ADD(m_In, byte_count);
    } else {
        unsigned int chunk = BITSTREAM_BUFFER_SIZE - m_In;
        if (chunk > byte_count) chunk = byte_count;

        CopyMemory(m_Buffer + m_In, bytes, chunk);
        BITSTREAM_POINTER_ADD(m_In, chunk);

        if (chunk != byte_count) {
            CopyMemory(m_Buffer + m_In,
                       bytes + chunk, byte_count - chunk);
            BITSTREAM_POINTER_ADD(m_In, byte_count - chunk);
        }
    }

    return SUCCESS;
}

Size BitStream::getContiguousBytesAvailable() {
    return
            (m_Out <= m_In) ?
            (m_In - m_Out) :
            (BITSTREAM_BUFFER_SIZE - m_Out);
}

Size BitStream::getBytesAvailable() {
    return
            (m_Out <= m_In) ?
            (m_In - m_Out) :
            (m_In + (BITSTREAM_BUFFER_SIZE - m_Out));
}

Result BitStream::readBytes(UI08 *bytes,
                            Size byte_count) {
    if (byte_count == 0 || bytes == NULL) {
        return ERROR_INVALID_PARAMETERS;
    }

    /* Gets bytes from the cache */
    byteAlign();
    while (m_BitsCached > 0 && byte_count > 0) {
        *bytes = (UI08) readBits(8);
        ++bytes;
        --byte_count;
    }

    /* get other bytes */
    if (byte_count > 0) {
        if (m_Out < m_In) {
            CopyMemory(bytes, m_Buffer + m_Out, byte_count);
            BITSTREAM_POINTER_ADD(m_Out, byte_count);
        } else {
            unsigned int chunk = BITSTREAM_BUFFER_SIZE - m_Out;
            if (chunk >= byte_count) chunk = byte_count;

            CopyMemory(bytes, m_Buffer + m_Out, chunk);
            BITSTREAM_POINTER_ADD(m_Out, chunk);

            if (chunk != byte_count) {
                CopyMemory(bytes + chunk,
                           m_Buffer + m_Out,
                           byte_count - chunk);
                BITSTREAM_POINTER_ADD(m_Out, byte_count - chunk);
            }
        }
    }

    return SUCCESS;
}

Result BitStream::peekBytes(UI08 *bytes,
                            Size byte_count) {
    int bits_cached_byte;

    if (byte_count == 0 || bytes == NULL) {
        return ERROR_INVALID_PARAMETERS;
    }

    /* Gets bytes from the cache */
    bits_cached_byte = m_BitsCached & ~7;
    while (bits_cached_byte > 0 && byte_count > 0) {
        *bytes = (m_Cache >> bits_cached_byte) & 0xFF;
        ++bytes;
        --byte_count;
        bits_cached_byte -= 8;
    }

    /* get other bytes */
    if (byte_count > 0) {
        if (m_In > m_Out) {
            CopyMemory(bytes, m_Buffer + m_Out, byte_count);
        } else {
            unsigned int out = m_Out;
            unsigned int chunk = BITSTREAM_BUFFER_SIZE - out;
            if (chunk >= byte_count) {
                chunk = byte_count;
            }

            CopyMemory(bytes, m_Buffer + out, chunk);
            BITSTREAM_POINTER_ADD(out, chunk);

            if (chunk != byte_count) {
                CopyMemory(bytes + chunk,
                           m_Buffer + out,
                           byte_count - chunk);
            }
        }
    }

    return SUCCESS;
}

Result BitStream::skipBytes(Size byte_count) {
    BITSTREAM_POINTER_ADD(m_Out, byte_count);
    return SUCCESS;
}

}
