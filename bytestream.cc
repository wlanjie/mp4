//
// Created by wlanjie on 2018/2/7.
//

#include "bytestream.h"
#include "utils.h"
#include "defined.h"

namespace mp4 {

const int BYTE_STREAM_COPY_BUFFER_SIZE = 65536;
const int MEMORY_BYTE_STREAM_MAX_SIZE  = 0x4000000; // 64 megs

Result ByteStream::read(void* buffer, Size bytesToRead) {
    // shortcut
    if (bytesToRead == 0) return SUCCESS;

    // read until failure
    Size bytes_read;
    while (bytesToRead) {
        Result result = readPartial(buffer, bytesToRead, bytes_read);
        if (FAILED(result)) return result;
        if (bytes_read == 0) return ERROR_INTERNAL;
        ASSERT(bytes_read <= bytesToRead);
        bytesToRead -= bytes_read;
        buffer = (void*)(((Byte*)buffer)+bytes_read);
    }

    return SUCCESS;
}

Result ByteStream::write(const void* buffer, Size bytesToWrite) {
    // shortcut
    if (bytesToWrite == 0) {
        return SUCCESS;
    }

    // write until failure
    Size bytes_written;
    while (bytesToWrite) {
        Result result = writePartial(buffer, bytesToWrite, bytes_written);
        if (FAILED(result)) {
            return result;
        }
        if (bytes_written == 0) {
            return ERROR_INTERNAL;
        }
        ASSERT(bytes_written <= bytesToWrite);
        bytesToWrite -= bytes_written;
        buffer = (const void*)(((const Byte*)buffer)+bytes_written);
    }

    return SUCCESS;
}

Result ByteStream::writeString(const char* buffer) {
    if (buffer == nullptr) {
        return SUCCESS;
    }

    auto string_length = static_cast<Size>(strlen(buffer));
    if (string_length == 0) {
        return SUCCESS;
    }

    // write the string
    return write((const void*)buffer, string_length);
}

Result ByteStream::writeDouble(double value) {
    unsigned char buffer[8];

    // convert value to bytes
    bytesFromDoubleBE(buffer, value);

    // write bytes to the stream
    return write((void*)buffer, 8);
}

Result ByteStream::writeUI64(UI64 value) {
    unsigned char buffer[8];

    // convert value to bytes
    bytesFromUInt64BE(buffer, value);

    // write bytes to the stream
    return write((void*)buffer, 8);
}

Result ByteStream::writeUI32(UI32 value) {
    unsigned char buffer[4];

    // convert value to bytes
    bytesFromUInt32BE(buffer, value);

    // write bytes to the stream
    return write((void*)buffer, 4);
}

Result ByteStream::writeUI24(UI32 value) {
    unsigned char buffer[3];

    // convert value to bytes
    bytesFromUInt24BE(buffer, value);

    // write bytes to the stream
    return write((void*)buffer, 3);
}

Result ByteStream::writeUI16(UI16 value) {
    unsigned char buffer[2];

    // convert value to bytes
    bytesFromUInt16BE(buffer, value);

    // write bytes to the stream
    return write((void*)buffer, 2);
}

Result ByteStream::writeUI08(UI08 value) {
    return write((void*)&value, 1);
}

Result ByteStream::readUI64(UI64& value) {
    unsigned char buffer[8];

    // read bytes from the stream
    Result result;
    result = read((void*)buffer, 8);
    if (FAILED(result)) {
        value = 0;
        return result;
    }

    // convert bytes to value
    value = bytesToUInt64BE(buffer);

    return SUCCESS;
}

Result ByteStream::readDouble(double& value) {
    unsigned char buffer[8];

    // read bytes from the stream
    Result result;
    result = read((void*)buffer, 8);
    if (FAILED(result)) {
        value = 0;
        return result;
    }

    // convert bytes to value
    value = bytesToDoubleBE(buffer);

    return SUCCESS;
}

Result ByteStream::readUI32(UI32& value) {
    unsigned char buffer[4];

    // read bytes from the stream
    Result result;
    result = read((void*)buffer, 4);
    if (FAILED(result)) {
        value = 0;
        return result;
    }

    // convert bytes to value
    value = bytesToUInt32BE(buffer);

    return SUCCESS;
}

Result ByteStream::readUI24(UI32& value) {
    unsigned char buffer[3];

    // read bytes from the stream
    Result result;
    result = read((void*)buffer, 3);
    if (FAILED(result)) {
        value = 0;
        return result;
    }

    // convert bytes to value
    value = bytesToUInt24BE(buffer);

    return SUCCESS;
}

Result ByteStream::readUI16(UI16& value) {
    unsigned char buffer[2];

    // read bytes from the stream
    Result result;
    result = read((void*)buffer, 2);
    if (FAILED(result)) {
        value = 0;
        return result;
    }

    // convert bytes to value
    value = bytesToUInt16BE(buffer);

    return SUCCESS;
}

Result ByteStream::readUI08(UI08& value) {
    unsigned char buffer[1];

    // read bytes from the stream
    Result result;
    result = read((void*)buffer, 1);
    if (FAILED(result)) {
        value = 0;
        return result;
    }

    // convert bytes to value
    value = buffer[0];

    return SUCCESS;
}

Result ByteStream::readString(char* buffer, Size size) {
    if (buffer == nullptr|| size == 0) {
        return ERROR_INVALID_PARAMETERS;
    }

    Size bytes_read = 0;
    while (bytes_read < size-1) {
        Result result;
        result = read(&buffer[bytes_read], 1);
        if (FAILED(result)) {
            buffer[bytes_read] = '\0';
            return result;
        }
        if (buffer[bytes_read] == '\0') {
            // end of string
            return SUCCESS;
        }
        bytes_read++;
    }

    // the string was not null terminated, terminate it
    buffer[size-1] = '\0';
    return SUCCESS;
}

Result ByteStream::readNullTerminatedString(String& string) {
    DataBuffer buffer;
    unsigned int size = 0;
    UI08 c = 0;
    do {
        Result result = readUI08(c);
        if (FAILED(result)) return result;
        buffer.setDataSize(size + 1);
        buffer.useData()[size] = c;
        ++size;
    } while (c);

    string.assign((const char *) buffer.getData(), size);
    return SUCCESS;
}

Result ByteStream::copyTo(ByteStream& stream, LargeSize size) {
    unsigned char buffer[BYTE_STREAM_COPY_BUFFER_SIZE];
    while (size) {
        Size bytes_read;
        Size bytes_to_read;
        Result result;

        // decide how much to read
        if (size >= sizeof(buffer)) {
            bytes_to_read = sizeof(buffer);
        } else {
            bytes_to_read = (Size)size;
        }

        // read up to one buffer full
        result = readPartial(buffer, bytes_to_read, bytes_read);
        if (FAILED(result)) {
            return result;
        }

        // copy to destination
        if (bytes_read != 0) {
            result = stream.write(buffer, bytes_read);
            if (FAILED(result)) {
                return result;
            }
        }

        // update the size
        size -= bytes_read;
    }

    return SUCCESS;
}

SubStream::SubStream(ByteStream& container,
                     Position    offset,
                     LargeSize   size) :
        container(container),
        offset(offset),
        size(size),
        position(0),
        referenceCount(1) {
    container.addReference();
}

SubStream::~SubStream() {
    container.release();
}

Result SubStream::readPartial(void* buffer,
                           Size bytesToRead,
                           Size& bytesRead) {
    // default values
    bytesRead = 0;

    // shortcut
    if (bytesToRead == 0) {
        return SUCCESS;
    }

    // clamp to range
    if (position+bytesToRead > size) {
        bytesToRead = (Size)(size - position);
    }

    // check for end of substream
    if (bytesToRead == 0) {
        return ERROR_EOS;
    }

    // seek inside container
    Result result;
    result = container.seek(offset+position);
    if (FAILED(result)) {
        return result;
    }

    // read from the container
    result = container.readPartial(buffer, bytesToRead, bytesRead);
    if (SUCCEEDED(result)) {
        position += bytesRead;
    }
    return result;
}

Result SubStream::writePartial(const void* buffer,
                            Size bytesToWrite,
                            Size& bytesWritten) {
    // default values
    bytesWritten = 0;

    // shortcut
    if (bytesToWrite == 0) {
        return SUCCESS;
    }

    // clamp to range
    if (position+bytesToWrite > size) {
        bytesToWrite = (Size)(size - position);
    }

    // check for end of substream
    if (bytesToWrite == 0) {
        return ERROR_EOS;
    }

    // seek inside container
    Result result;
    result = container.seek(offset+position);
    if (FAILED(result)) return result;

    // write to container
    result = container.writePartial(buffer, bytesToWrite, bytesWritten);
    if (SUCCEEDED(result)) {
        position += bytesWritten;
    }
    return result;
}

Result SubStream::seek(Position position) {
    if (this->position == position) {
        return SUCCESS;
    }
    if (position > size) {
        return FAILURE;
    }
    this->position = position;
    return SUCCESS;
}

void SubStream::addReference() {
    referenceCount++;
}

void SubStream::release() {
    if (--referenceCount == 0) {
        delete this;
    }
}

MemoryByteStream::MemoryByteStream(Size size) :
        m_BufferIsLocal(true),
        m_Position(0),
        m_ReferenceCount(1) {
    m_Buffer = new DataBuffer(size);
    SetMemory(m_Buffer->useData(), 0, size);
    m_Buffer->setDataSize(size);
}

MemoryByteStream::MemoryByteStream(const UI08* buffer, Size size) :
        m_BufferIsLocal(true),
        m_Position(0),
        m_ReferenceCount(1) {
    m_Buffer = new DataBuffer(buffer, size);
}

MemoryByteStream::MemoryByteStream(DataBuffer& data_buffer) :
        m_Buffer(&data_buffer),
        m_BufferIsLocal(false),
        m_Position(0),
        m_ReferenceCount(1) {
}

MemoryByteStream::MemoryByteStream(DataBuffer* data_buffer) :
        m_Buffer(data_buffer),
        m_BufferIsLocal(true),
        m_Position(0),
        m_ReferenceCount(1) {
}

MemoryByteStream::~MemoryByteStream() {
    if (m_BufferIsLocal) {
        delete m_Buffer;
    }
}

Result MemoryByteStream::readPartial(void* buffer, Size bytesToRead, Size& bytesRead) {
    // default values
    bytesRead = 0;

    // shortcut
    if (bytesToRead == 0) {
        return SUCCESS;
    }

    // clamp to range
    if (m_Position+bytesToRead > m_Buffer->getDataSize()) {
        bytesToRead = (Size)(m_Buffer->getDataSize() - m_Position);
    }

    // check for end of stream
    if (bytesToRead == 0) {
        return ERROR_EOS;
    }

    // read from the memory
    CopyMemory(buffer, m_Buffer->getData()+m_Position, bytesToRead);
    m_Position += bytesToRead;

    bytesRead = bytesToRead;

    return SUCCESS;
}

Result MemoryByteStream::writePartial(const void* buffer, Size bytesToWrite, Size& bytesWritten) {
    // default values
    bytesWritten = 0;

    // shortcut
    if (bytesToWrite == 0) {
        return SUCCESS;
    }

    // check that we don't exceed the max
    if (m_Position+bytesToWrite > (Position)MEMORY_BYTE_STREAM_MAX_SIZE) {
        return ERROR_OUT_OF_RANGE;
    }

    // reserve space in the buffer
    auto space_needed = (Size)(m_Position+bytesToWrite);
    Result result = m_Buffer->reserve(space_needed);
    if (SUCCEEDED(result)) {
        if (space_needed > m_Buffer->getDataSize()) {
            // the buffer must grow
            m_Buffer->setDataSize(space_needed);
        }
    } else {
        // failed to reserve, most likely caused by a buffer that has
        // external storage
        if (m_Position+bytesToWrite > m_Buffer->getDataSize()) {
            bytesToWrite = (Size)(m_Buffer->getDataSize() - m_Position);
        }
    }

    // check for end of stream
    if (bytesToWrite == 0) {
        return ERROR_EOS;
    }

    // write to memory
    CopyMemory((void*)(m_Buffer->useData()+m_Position), buffer, bytesToWrite);
    m_Position += bytesToWrite;

    bytesWritten = bytesToWrite;

    return SUCCESS;
}

Result MemoryByteStream::seek(Position position) {
    if (position > m_Buffer->getDataSize()) return FAILURE;
    m_Position = position;
    return SUCCESS;
}

void MemoryByteStream::addReference() {
    m_ReferenceCount++;
}

void MemoryByteStream::release() {
    if (--m_ReferenceCount == 0) {
        delete this;
    }
}

}
