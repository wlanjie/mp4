//
// Created by wlanjie on 2018/3/15.
//

#include "decoder_specificinfo_descriptor.h"

namespace mp4 {

DecoderSpecificInfoDescriptor::DecoderSpecificInfoDescriptor(const DataBuffer &data) :
        Descriptor(DESCRIPTOR_TAG_DECODER_SPECIFIC_INFO, minHeaderSize(data.getDataSize()), data.getDataSize()),
        info(data) {

}

DecoderSpecificInfoDescriptor::DecoderSpecificInfoDescriptor(ByteStream &stream, Size size, Size payloadSize) :
        Descriptor(DESCRIPTOR_TAG_DECODER_SPECIFIC_INFO, size, payloadSize) {
    info.setDataSize(payloadSize);
    stream.read(info.useData(), payloadSize);
}

DecoderSpecificInfoDescriptor::~DecoderSpecificInfoDescriptor() {

}

Result DecoderSpecificInfoDescriptor::writeFields(ByteStream &stream) {
    if (payloadSize && info.getDataSize()) {
        stream.write(info.getData(), info.getDataSize());
    }
    return SUCCESS;
}

}
