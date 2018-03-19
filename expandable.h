//
// Created by wlanjie on 2018/3/15.
//

#ifndef MP4_EXPANDABLE_H
#define MP4_EXPANDABLE_H


#include "types.h"
#include "bytestream.h"
#include "atom.h"

class Expandable {
public:
    enum ClassIdSize {
        CLASS_ID_SIZE_08
    };

    static Size minHeaderSize(Size payloadSize);
    Expandable(UI32 classId, ClassIdSize classIdSize, Size headerSize, Size payloadSize);
    virtual ~Expandable() {};
    UI32 getClassId() { return classId; }
    Size getSize() { return payloadSize + headerSize; }
    Size getHeaderSize() { return headerSize; }
    virtual Result write(ByteStream& stream);
    virtual Result writeFields(ByteStream& stream) = 0;

protected:
    UI32 classId;
    ClassIdSize classIdSize;
    Size headerSize;
    Size payloadSize;
};


#endif //MP4_EXPANDABLE_H
