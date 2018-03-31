//
// Created by wlanjie on 2018/3/7.
//

#ifndef MP4_FILEBYTESTREAM_H
#define MP4_FILEBYTESTREAM_H

#include "bytestream.h"

namespace mp4 {

class FileByteStream : public ByteStream {
public:
    typedef enum {
        STREAM_MODE_READ        = 0,
        STREAM_MODE_WRITE       = 1,
        STREAM_MODE_READ_WRITE  = 2
    } Mode;

    static Result create(const char* name, Mode mode, ByteStream*& stream);
    FileByteStream(ByteStream* delegate) : delegate(delegate){}
    FileByteStream(const char* name, Mode mode);
    Result readPartial(void* buffer, Size bytesToRead, Size& bytesRead) {
        return delegate->readPartial(buffer, bytesToRead, bytesRead);
    }
    Result writePartial(const void* buffer, Size bytesToWrite, Size& bytesWritten) {
        return delegate->writePartial(buffer, bytesToWrite, bytesToWrite);
    }
    Result seek(Position position) { return delegate->seek(position); }
    Result tell(Position& position) { return delegate->tell(position); }
    Result getSize(LargeSize& size) { return delegate->getSize(size); }
//    Result flush() { return delegate->flush(); }
    void addReference() { delegate->addReference(); }
    void release() { delegate->release(); }
protected:
    virtual ~FileByteStream() {
        delete delegate;
    }
    ByteStream* delegate;
};

}
#endif //MP4_FILEBYTESTREAM_H
