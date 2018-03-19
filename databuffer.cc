//
// Created by wlanjie on 2018/2/7.
//

#include "databuffer.h"
#include "utils.h"

/*----------------------------------------------------------------------
|   DataBuffer::DataBuffer
+---------------------------------------------------------------------*/
DataBuffer::DataBuffer() :
        m_BufferIsLocal(true),
        m_Buffer(NULL),
        m_BufferSize(0),
        m_DataSize(0)
{
}

/*----------------------------------------------------------------------
|   DataBuffer::DataBuffer
+---------------------------------------------------------------------*/
DataBuffer::DataBuffer(Size buffer_size) :
        m_BufferIsLocal(true),
        m_Buffer(NULL),
        m_BufferSize(buffer_size),
        m_DataSize(0)
{
    m_Buffer = new Byte[buffer_size];
}

/*----------------------------------------------------------------------
|   DataBuffer::DataBuffer
+---------------------------------------------------------------------*/
DataBuffer::DataBuffer(const void* data, Size data_size) :
        m_BufferIsLocal(true),
        m_Buffer(NULL),
        m_BufferSize(data_size),
        m_DataSize(data_size)
{
    if (data && data_size) {
        m_Buffer = new Byte[data_size];
        CopyMemory(m_Buffer, data, data_size);
    }
}

/*----------------------------------------------------------------------
|   DataBuffer::DataBuffer
+---------------------------------------------------------------------*/
DataBuffer::DataBuffer(const DataBuffer& other) :
        m_BufferIsLocal(true),
        m_Buffer(NULL),
        m_BufferSize(other.m_DataSize),
        m_DataSize(other.m_DataSize)
{
    m_Buffer = new Byte[m_BufferSize];
    CopyMemory(m_Buffer, other.m_Buffer, m_BufferSize);
}

/*----------------------------------------------------------------------
|   DataBuffer::~DataBuffer
+---------------------------------------------------------------------*/
DataBuffer::~DataBuffer()
{
    if (m_BufferIsLocal) {
        delete[] m_Buffer;
    }
}

/*----------------------------------------------------------------------
|   DataBuffer::reserve
+---------------------------------------------------------------------*/
Result
DataBuffer::reserve(Size size)
{
    if (size <= m_BufferSize) return SUCCESS;

    // try doubling the buffer to accomodate for the new size
    Size new_size = m_BufferSize*2+1024;
    if (new_size < size) new_size = size;
    return setBufferSize(new_size);
}

/*----------------------------------------------------------------------
|   DataBuffer::setBuffer
+---------------------------------------------------------------------*/
Result
DataBuffer::setBuffer(Byte *buffer, Size buffer_size)
{
    if (m_BufferIsLocal) {
        // destroy the local buffer
        delete[] m_Buffer;
    }

    // we're now using an external buffer
    m_BufferIsLocal = false;
    m_Buffer = buffer;
    m_BufferSize = buffer_size;

    return SUCCESS;
}

/*----------------------------------------------------------------------
|   DataBuffer::setBufferSize
+---------------------------------------------------------------------*/
Result
DataBuffer::setBufferSize(Size buffer_size)
{
    if (m_BufferIsLocal) {
        return reallocateBuffer(buffer_size);
    } else {
        return FAILURE; // you cannot change the
        // buffer management mode
    }
}

/*----------------------------------------------------------------------
|   DataBuffer::setDataSize
+---------------------------------------------------------------------*/
Result
DataBuffer::setDataSize(Size size)
{
    if (size > m_BufferSize) {
        if (m_BufferIsLocal) {
            Result result = reallocateBuffer(size);
            if (FAILED(result)) return result;
        } else {
            return FAILURE;
        }
    }
    m_DataSize = size;
    return SUCCESS;
}

/*----------------------------------------------------------------------
|   DataBuffer::setData
+---------------------------------------------------------------------*/
Result
DataBuffer::setData(const Byte *data, Size size)
{
    if (size > m_BufferSize) {
        if (m_BufferIsLocal) {
            Result result = reallocateBuffer(size);
            if (FAILED(result)) return result;
        } else {
            return FAILURE;
        }
    }
    CopyMemory(m_Buffer, data, size);
    m_DataSize = size;

    return SUCCESS;
}

/*----------------------------------------------------------------------
|   DataBuffer::appendData
+---------------------------------------------------------------------*/
Result
DataBuffer::appendData(const Byte *data, Size data_size)
{
    if (data == NULL || data_size == 0) {
        return SUCCESS;
    }

    Size existing_size = m_DataSize;
    Result result = setDataSize(existing_size + data_size);
    if (FAILED(result)) {
        return result;
    }
    CopyMemory(m_Buffer+existing_size, data, data_size);
    return SUCCESS;
}

/*----------------------------------------------------------------------
|   DataBuffer::reallocateBuffer
+---------------------------------------------------------------------*/
Result
DataBuffer::reallocateBuffer(Size size)
{
    // check that the existing data fits
    if (m_DataSize > size) return FAILURE;

    // allocate a new buffer
    Byte* new_buffer = new Byte[size];

    // copy the contents of the previous buffer ,is any
    if (m_Buffer && m_DataSize) {
        CopyMemory(new_buffer, m_Buffer, m_DataSize);
    }

    // destroy the previous buffer
    delete[] m_Buffer;

    // use the new buffer
    m_Buffer = new_buffer;
    m_BufferSize = size;

    return SUCCESS;
}