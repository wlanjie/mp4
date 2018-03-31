//
// Created by wlanjie on 2018/3/16.
//

#ifndef MP4_SLCONFIGDESCRIPTOR_H
#define MP4_SLCONFIGDESCRIPTOR_H

#include "descriptor.h"

namespace mp4 {

const UI08 DESCRIPTOR_TAG_SL_CONFIG = 0x06;

class SLConfigDescriptor : public Descriptor {
public:
    SLConfigDescriptor(Size headerSize = 2);
    virtual Result writeFields(ByteStream& stream);

private:
    UI08 predefined;
};

}
#endif //MP4_SLCONFIGDESCRIPTOR_H
