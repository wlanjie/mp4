//
// Created by wlanjie on 2018/3/15.
//

#ifndef MP4_DESCRIPTORFACTORY_H
#define MP4_DESCRIPTORFACTORY_H


#include "types.h"
#include "bytestream.h"
#include "descriptor.h"

class DescriptorFactory {
public:
    static Result createDescriptionFromStream(ByteStream& stream, Descriptor*& descriptor);
};


#endif //MP4_DESCRIPTORFACTORY_H
