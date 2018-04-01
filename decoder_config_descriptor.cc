//
// Created by wlanjie on 2018/3/15.
//

#include "decoder_config_descriptor.h"
#include "descriptor_factory.h"

namespace mp4 {

DecoderConfigDescriptor::DecoderConfigDescriptor(UI08 streamType,
                                                 UI08 oti,
                                                 UI32 bufferSize,
                                                 UI32 maxBitRate,
                                                 UI32 avgBitRate,
                                                 DecoderSpecificInfoDescriptor *dsi) :
        Descriptor(DESCRIPTOR_TAG_DECODER_CONFIG, 2, 13),
        streamType(streamType),
        objectTypeIndication(oti),
        upStream(false),
        bufferSize(bufferSize),
        maxBitRate(maxBitRate),
        averageBitRate(avgBitRate) {
    if (dsi) {
        subDescriptors.add(dsi);
        payloadSize += dsi->getSize();
        headerSize = minHeaderSize(payloadSize);
    }
}

DecoderConfigDescriptor::DecoderConfigDescriptor(ByteStream &stream, Size headerSize, Size payloadSize) :
        Descriptor(DESCRIPTOR_TAG_DECODER_CONFIG, headerSize, payloadSize) {
    Position start;
    stream.tell(start);

    stream.readUI08(objectTypeIndication);
    unsigned char bits;
    stream.readUI08(bits);
    streamType = (bits >> 2) & 0x3F;
    upStream = bits & 2 ? true : false;
    stream.readUI24(bufferSize);
    stream.readUI32(maxBitRate);
    stream.readUI32(averageBitRate);

    auto* subStream = new SubStream(stream, start + 13, payloadSize);
    Descriptor* descriptor = nullptr;
    while (DescriptorFactory::createDescriptionFromStream(*subStream, descriptor) == SUCCESS) {
        subDescriptors.add(descriptor);
    }
    subStream->release();
}

DecoderConfigDescriptor::~DecoderConfigDescriptor() {
    subDescriptors.deleteReferences();
}

Result DecoderConfigDescriptor::writeFields(ByteStream &stream) {
    stream.writeUI08(objectTypeIndication);
    UI08 bits = (streamType << 2) | (upStream ? 2 : 0) | 1;
    stream.writeUI08(bits);
    stream.writeUI24(bufferSize);
    stream.writeUI32(maxBitRate);
    stream.writeUI32(averageBitRate);
    subDescriptors.apply(DescriptorListWriter(stream));
    return 0;
}

const DecoderSpecificInfoDescriptor *DecoderConfigDescriptor::getDecoderSpecificInfoDescriptor() const {
    Descriptor* descriptor = nullptr;
    auto result = subDescriptors.find(DescriptorFinder(DESCRIPTOR_TAG_DECODER_SPECIFIC_INFO), descriptor);
    return SUCCEEDED(result) ? DYNAMIC_CAST(DecoderSpecificInfoDescriptor, descriptor) : nullptr;
}

}
