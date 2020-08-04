//
// Created by wlanjie on 2018/2/7.
//

#ifndef MP4_BYTESTREAM_H
#define MP4_BYTESTREAM_H

#include "interfaces.h"
#include "types.h"
#include "character.h"
#include "results.h"
#include "databuffer.h"

namespace mp4 {

class ByteStream : public Referenceable {
public:
    virtual Result readPartial(void *buffer, Size bytesToRead, Size& bytesRead) = 0;
    Result read(void* buffer, Size bytesToRead);
    Result readDouble(double& value);
    Result readUI64(UI64& value);
    Result readUI32(UI32& value);
    Result readUI24(UI32& value);
    Result readUI16(UI16& value);
    Result readUI08(UI08& value);
    Result readString(char* buffer, Size size);
    Result readNullTerminatedString(String& string);
    virtual Result writePartial(const void* buffer, Size bytesToWrite, Size& bytesWritten) = 0;
    Result write(const void* buffer, Size bytesToWrite);
    Result writeString(const char* buffer);
    Result writeDouble(double value);
    Result writeUI64(UI64 value);
    Result writeUI32(UI32 value);
    Result writeUI24(UI32 value);
    Result writeUI16(UI16 value);
    Result writeUI08(UI08 value);
    virtual Result seek(Position position) = 0;
    virtual Result tell(Position& position) = 0;
    virtual Result getSize(LargeSize& size) = 0;
    virtual Result copyTo(ByteStream& stream, LargeSize size);
};

class SubStream : public ByteStream {
public:
    SubStream(ByteStream& container, Position position, LargeSize size);
    Result readPartial(void* buffer, Size bytesToRead, Size& bytesRead);
    Result writePartial(const void* buffer, Size bytesToWrite, Size& bytesWritten);
    Result seek(Position position);
    Result tell(Position& position) {
        position = this->position;
        return SUCCESS;
    }
    Result getSize(LargeSize& size) {
        this->size = size;
        return SUCCESS;
    }
    void addReference();
    void release();

protected:
    virtual ~SubStream();

private:
    ByteStream& container;
    Position position;
    LargeSize size;
    Position offset;
    Cardinal referenceCount;
};

/*----------------------------------------------------------------------
|   MemoryByteStream
+---------------------------------------------------------------------*/
class MemoryByteStream : public ByteStream
{
public:
    MemoryByteStream(Size size = 0); // filled with zeros
    MemoryByteStream(const UI08* buffer, Size size);
    MemoryByteStream(DataBuffer& data_buffer); // data is read/written from/to supplied buffer, no ownership transfer
    MemoryByteStream(DataBuffer* data_buffer); // data is read/written from/to supplied buffer, ownership is transfered

    // ByteStream methods
    Result readPartial(void*     buffer,
                           Size  bytes_to_read,
                           Size& bytes_read);
    Result writePartial(const void* buffer,
                            Size    bytes_to_write,
                            Size&   bytes_written);
    Result seek(Position position);
    Result tell(Position& position) {
        position = m_Position;
        return SUCCESS;
    }
    Result getSize(LargeSize& size) {
        size = m_Buffer->getDataSize();
        return SUCCESS;
    }

    // Referenceable methods
    void addReference();
    void release();

    // methods
    const UI08* getData()     { return m_Buffer->getData(); }
    UI08*       useData()     { return m_Buffer->useData(); }
    Size        getDataSize() { return m_Buffer->getDataSize(); }

protected:
    virtual ~MemoryByteStream();

private:
    DataBuffer* m_Buffer;
    bool        m_BufferIsLocal;
    Position    m_Position;
    Cardinal    m_ReferenceCount;
};

}
#endif //MP4_BYTESTREAM_H
