//
// Created by wlanjie on 2018/3/15.
//

#include "expandable.h"
#include "utils.h"

namespace mp4 {

Size Expandable::minHeaderSize(Size payloadSize) {
    return 2 + (payloadSize / 128);
}

Expandable::Expandable(UI32 classId,
                       Expandable::ClassIdSize classIdSize,
                       Size headerSize,
                       Size payloadSize) :
        classId(classId),
        classIdSize(classIdSize),
        headerSize(headerSize),
        payloadSize(payloadSize) {
    ASSERT(headerSize >= 1 + 1);
    ASSERT(headerSize <= 1 + 4);
}

Result Expandable::write(ByteStream &stream) {
    Result result;
    switch (classIdSize) {
        case CLASS_ID_SIZE_08:
            result = stream.writeUI08((UI08) (classId));
            if (FAILED(result)) {
                return result;
            }
            break;

        default:
            return ERROR_INTERNAL;
    }
    ASSERT(headerSize - 1 <= 8);
    ASSERT(headerSize >= 2);
    unsigned int size = payloadSize;
    unsigned char bytes[8];

    // last bytes of the encoded size
    bytes[headerSize-2] = size&0x7F;

    // leading bytes of the encoded size
    for (int i = headerSize - 3; i >= 0; i--) {
        // move to the next 7 bits
        size >>= 7;

        // output a byte with a top bit marker
        bytes[i] = (size&0x7F) | 0x80;
    }

    result = stream.write(bytes, headerSize - 1);
    if (FAILED(result)) {
        return result;
    }

    // write the fields
    writeFields(stream);
    return SUCCESS;
}

}
