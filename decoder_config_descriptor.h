//
// Created by wlanjie on 2018/3/15.
//

#ifndef MP4_DECODERCONFIGDESCRIPTOR_H
#define MP4_DECODERCONFIGDESCRIPTOR_H


#include "decoder_specificinfo_descriptor.h"

const UI08 DESCRIPTOR_TAG_DECODER_CONFIG = 0x04;

class DecoderConfigDescriptor : public Descriptor {
public:
    DecoderConfigDescriptor(UI08 streamType, UI08 oti, UI32 bufferSize, UI32 maxBitRate, UI32 avgBitRate, DecoderSpecificInfoDescriptor* dsi);
    DecoderConfigDescriptor(ByteStream& stream, Size headerSize, Size payloadSize);
    virtual ~DecoderConfigDescriptor();
    virtual Result writeFields(ByteStream& stream);

    virtual const DecoderSpecificInfoDescriptor* getDecoderSpecificInfoDescriptor() const;
    virtual UI08 getObjectTypeIndication() const { return objectTypeIndication; }
    virtual UI08 getStreamType() const { return streamType; }
    virtual UI32 getBufferSize() const { return bufferSize; }
    virtual UI32 getMaxBitRate() const { return maxBitRate; }
    virtual UI32 getAvgBitRate() const { return averageBitRate; }
private:
    UI08 streamType;
    UI08 objectTypeIndication;
    bool upStream;
    UI32 bufferSize;
    UI32 maxBitRate;
    UI32 averageBitRate;
    mutable List<Descriptor> subDescriptors;

};


#endif //MP4_DECODERCONFIGDESCRIPTOR_H
