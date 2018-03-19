//
// Created by wlanjie on 2018/3/15.
//

#ifndef MP4_DECODERSPECIFICINFODESCRIPTOR_H
#define MP4_DECODERSPECIFICINFODESCRIPTOR_H


#include "descriptor.h"

const UI08 DESCRIPTOR_TAG_DECODER_SPECIFIC_INFO = 0x05;

class DecoderSpecificInfoDescriptor : public Descriptor {
public:
    DecoderSpecificInfoDescriptor(const DataBuffer& data);
    DecoderSpecificInfoDescriptor(ByteStream& stream, Size size, Size payloadSize);

    virtual ~DecoderSpecificInfoDescriptor();
    virtual Result writeFields(ByteStream& stream);
    virtual const DataBuffer& getDecoderSpecificInfo() const { return info; }

private:
    DataBuffer info;
};


#endif //MP4_DECODERSPECIFICINFODESCRIPTOR_H
