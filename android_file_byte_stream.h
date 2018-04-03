//
// Created by wlanjie on 2018/4/3.
//

#ifndef MP4_ANDROID_FILE_BYTE_STREAM_H
#define MP4_ANDROID_FILE_BYTE_STREAM_H

#include "filebytestream.h"
#include "bytestream.h"
#include "types.h"

namespace mp4 {
class AndroidFileByteStream : public ByteStream {
public:
    static Result create(FileByteStream* delegator,
                         const char* name,
                         FileByteStream::Mode mode,
                         ByteStream*& stream);

    AndroidFileByteStream(FileByteStream* delegator,
                          int fd,
                          LargeSize size);

    ~AndroidFileByteStream() override;

    Result readPartial(void* buffer,
                       Size bytesToRead,
                       Size& bytesRead) override;
    Result writePartial(const void* buffer,
                        Size bytesToWrite,
                        Size& bytesWritten) override;
    Result seek(Position position) override;
    Result tell(Position& position) override;
    Result getSize(LargeSize& size) override;
    Result flush();

    void addReference() override;
    void release() override;

private:
    // members
    ByteStream* delegator;
    Cardinal referenceCount;
    int fd;
    Position position;
    LargeSize size;
};

}
#endif //MP4_ANDROID_FILE_BYTE_STREAM_H
