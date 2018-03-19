//
// Created by wlanjie on 2018/2/7.
//

#ifndef MP4_DATABUFFER_H
#define MP4_DATABUFFER_H

#include "types.h"
#include "results.h"

class DataBuffer {
public:
    // constructors & destructor
    DataBuffer();

    DataBuffer(Size size);

    DataBuffer(const void *data, Size data_size);

    DataBuffer(const DataBuffer &other);

    virtual ~DataBuffer();

    // data buffer handling methods
    Result setBuffer(Byte *buffer, Size buffer_size);

    Result setBufferSize(Size buffer_size);

    Size getBufferSize() const { return m_BufferSize; }

    // data handling methods
    const Byte *getData() const { return m_Buffer; }

    Byte *useData() { return m_Buffer; };

    Size getDataSize() const { return m_DataSize; }

    Result setDataSize(Size size);

    Result setData(const Byte *data, Size data_size);

    Result appendData(const Byte *data, Size data_size);

    // memory management
    Result reserve(Size size);

protected:
    // members
    bool m_BufferIsLocal;
    Byte *m_Buffer;
    Size m_BufferSize;
    Size m_DataSize;

    // methods
    Result reallocateBuffer(Size size);

private:
    // forbid this
    DataBuffer &operator=(const DataBuffer &other);
};


#endif //MP4_DATABUFFER_H
