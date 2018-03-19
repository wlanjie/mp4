//
// Created by wlanjie on 2018/3/15.
//

#include "descriptor_factory.h"
#include "decoder_specificinfo_descriptor.h"
#include "decoder_config_descriptor.h"
#include "es_descriptor.h"

Result DescriptorFactory::createDescriptionFromStream(ByteStream &stream, Descriptor *&descriptor) {
    Result result;
    descriptor = nullptr;
    Position offset;
    stream.tell(offset);

    unsigned char tag;
    result = stream.readUI08(tag);
    if (FAILED(result)) {
        stream.seek(offset);
        return result;
    }
    UI32 payloadSize = 0;
    unsigned int headerSize = 1;
    unsigned int max = 4;
    unsigned char ext = 0;

    do {
        headerSize++;
        result = stream.readUI08(ext);
        if (FAILED(result)) {
            stream.seek(offset);
            return result;
        }
        payloadSize = (payloadSize << 7) + (ext & 0x7F);
    } while (--max && (ext & 0x80));
    switch (tag) {
        case DESCRIPTOR_TAG_ES:
            descriptor = new EsDescriptor(stream, headerSize, payloadSize);
            break;

        case DESCRIPTOR_TAG_DECODER_CONFIG:
            descriptor = new DecoderConfigDescriptor(stream, headerSize, payloadSize);
            break;

        case DESCRIPTOR_TAG_DECODER_SPECIFIC_INFO:
            descriptor = new DecoderSpecificInfoDescriptor(stream, headerSize, payloadSize);
            break;
    }
    stream.seek(offset + headerSize + payloadSize);
    return 0;
}
