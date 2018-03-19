/*****************************************************************
|
|    AP4 - Bitstream Utility
|
|    Copyright 2002-2008 Axiomatic Systems, LLC
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
****************************************************************/

/*----------------------------------------------------------------------
|   For efficiency reasons, this bitstream library only handles
|   data buffers that are a power of 2 in size
+---------------------------------------------------------------------*/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "bit_stream.h"
#include "utils.h"

/*----------------------------------------------------------------------
|   BitStream::BitStream
+---------------------------------------------------------------------*/
BitStream::BitStream() {
    m_Buffer = new UI08[BITSTREAM_BUFFER_SIZE];
    Reset();
}

/*----------------------------------------------------------------------
|   BitStream::~BitStream
+---------------------------------------------------------------------*/
BitStream::~BitStream() {
    delete[] m_Buffer;
}

/*----------------------------------------------------------------------
|   BitStream::Reset
+---------------------------------------------------------------------*/
Result
BitStream::Reset() {
    m_In = 0;
    m_Out = 0;
    m_BitsCached = 0;
    m_Cache = 0;
    m_Flags = 0;

    return SUCCESS;
}

/*----------------------------------------------------------------------
|   BitStream::ByteAlign
+---------------------------------------------------------------------*/
Result
BitStream::ByteAlign() {
    unsigned int to_flush = m_BitsCached & 7;
    if (to_flush > 0) SkipBits(to_flush);

    return SUCCESS;
}

/*----------------------------------------------------------------------
|   BitStream::GetContiguousBytesFree
+---------------------------------------------------------------------*/
Size
BitStream::GetContiguousBytesFree() {
    return
            (m_In < m_Out) ?
            (m_Out - m_In - 1) :
            (m_Out == 0 ? (BITSTREAM_BUFFER_SIZE - m_In - 1) :
             (BITSTREAM_BUFFER_SIZE - m_In));
}

/*----------------------------------------------------------------------
|   BitStream_GetBytesFree
+---------------------------------------------------------------------*/
Size
BitStream::GetBytesFree() {
    return
            (m_In < m_Out) ?
            (m_Out - m_In - 1) :
            (BITSTREAM_BUFFER_SIZE + (m_Out - m_In) - 1);
}

/*----------------------------------------------------------------------+
|    BitStream::WriteBytes
+----------------------------------------------------------------------*/
Result
BitStream::WriteBytes(const UI08 *bytes,
                      Size byte_count) {
    /* check parameters */
    if (byte_count == 0) return SUCCESS;
    if (bytes == NULL) return ERROR_INVALID_PARAMETERS;

    /* check that we have enough space */
    if (GetBytesFree() < byte_count) {
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

/*----------------------------------------------------------------------
|   BitStream_GetContiguousBytesAvailable
+---------------------------------------------------------------------*/
Size
BitStream::GetContiguousBytesAvailable() {
    return
            (m_Out <= m_In) ?
            (m_In - m_Out) :
            (BITSTREAM_BUFFER_SIZE - m_Out);
}

/*----------------------------------------------------------------------
|   BitStream::GetBytesAvailable
+---------------------------------------------------------------------*/
Size
BitStream::GetBytesAvailable() {
    return
            (m_Out <= m_In) ?
            (m_In - m_Out) :
            (m_In + (BITSTREAM_BUFFER_SIZE - m_Out));
}

/*----------------------------------------------------------------------+
|    BitStream::ReadBytes
+----------------------------------------------------------------------*/
Result
BitStream::ReadBytes(UI08 *bytes,
                     Size byte_count) {
    if (byte_count == 0 || bytes == NULL) {
        return ERROR_INVALID_PARAMETERS;
    }

    /* Gets bytes from the cache */
    ByteAlign();
    while (m_BitsCached > 0 && byte_count > 0) {
        *bytes = (UI08) ReadBits(8);
        ++bytes;
        --byte_count;
    }

    /* Get other bytes */
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

/*----------------------------------------------------------------------+
|    BitStream::PeekBytes
+----------------------------------------------------------------------*/
Result
BitStream::PeekBytes(UI08 *bytes,
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

    /* Get other bytes */
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

/*----------------------------------------------------------------------+
|    BitStream::SkipBytes
+----------------------------------------------------------------------*/
Result
BitStream::SkipBytes(Size byte_count) {
    BITSTREAM_POINTER_ADD(m_Out, byte_count);
    return SUCCESS;
}
