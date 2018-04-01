//
// Created by wlanjie on 2018/3/16.
//

#include "es_descriptor.h"
#include "descriptor_factory.h"

namespace mp4 {

EsDescriptor::EsDescriptor(UI16 esId) :
        Descriptor(DESCRIPTOR_TAG_ES, 2, 2 + 1),
        esId(esId),
        ocrEsId(0),
        flags(0),
        streamPriority(0),
        dependsOn(0) {
    return;
}

EsDescriptor::EsDescriptor(ByteStream &stream, Size headerSize, Size payloadSize) :
        Descriptor(DESCRIPTOR_TAG_ES, headerSize, payloadSize) {

    Position start;
    stream.tell(start);

    // read descriptor fields
    stream.readUI16(esId);
    unsigned char bits;
    stream.readUI08(bits);
    flags = (bits >> 5) & 7;
    streamPriority = bits & 0x1F;
    if (flags & ES_DESCRIPTOR_FLAG_STREAM_DEPENDENCY) {
        stream.readUI16(dependsOn);
    }  else {
        dependsOn = 0;
    }
    if (flags & ES_DESCRIPTOR_FLAG_URL) {
        unsigned char urlLength;
        stream.readUI08(urlLength);
        if (urlLength) {
            auto* url = new char[urlLength + 1];
            if (url) {
                stream.read(url, urlLength);
                url[urlLength] = '\0';
                this->url = url;
                delete[] url;
            }
        }
    }
    if (flags & ES_DESCRIPTOR_FLAG_URL) {
        stream.readUI16(ocrEsId);
    } else {
        ocrEsId = 0;
    }

    // read other descriptors
    Position offset;
    stream.tell(offset);
    auto* substream = new SubStream(stream, offset, payloadSize - Size(offset - start));
    Descriptor* descriptor = nullptr;
    while (DescriptorFactory::createDescriptionFromStream(*substream, descriptor) == SUCCESS) {
        subDescriptors.add(descriptor);
    }
    substream->release();
}

EsDescriptor::~EsDescriptor() {
    subDescriptors.deleteReferences();
}

Result EsDescriptor::addSubDescriptor(Descriptor *desciptor) {
    subDescriptors.add(desciptor);
    payloadSize += desciptor->getSize();
    return SUCCESS;
}

Result EsDescriptor::writeFields(ByteStream &stream) {

    Result result;

    // es id
    result = stream.writeUI16(esId);
    if (FAILED(result)) {
        return result;
    }

    // flags and other bits
    UI08 bits = streamPriority | (UI08)(flags << 5);
    result = stream.writeUI08(bits);
    if (FAILED(result)) {
        return result;
    }

    // optional fields
    if (flags & ES_DESCRIPTOR_FLAG_STREAM_DEPENDENCY) {
        result = stream.writeUI16(dependsOn);
        if (FAILED(result)) {
            return result;
        }
    }
    if (flags & ES_DESCRIPTOR_FLAG_URL) {
        result = stream.writeUI08((UI08) url.getLength());
        if (FAILED(result)) {
            return result;
        }
        result = stream.writeString(url.getChars());
        if (FAILED(result)) {
            return result;
        }
        result = stream.writeUI08(0);
        if (FAILED(result)) {
            return result;
        }
    }
    if (flags & ES_DESCRIPTOR_FLAG_OCR_STREAM) {
        result = stream.writeUI16(ocrEsId);
        if (FAILED(result)) {
            return result;
        }
    }

    // write the sub descriptors
    subDescriptors.apply(DescriptorListWriter(stream));
    return SUCCESS;
}

const DecoderConfigDescriptor *EsDescriptor::getDecoderConfigDescription() const {
    Descriptor* descriptor = nullptr;
    auto result = subDescriptors.find(DescriptorFinder(DESCRIPTOR_TAG_DECODER_CONFIG), descriptor);
    return SUCCEEDED(result) ? DYNAMIC_CAST(DecoderConfigDescriptor, descriptor) : nullptr;
}

}
