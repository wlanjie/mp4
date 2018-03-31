//
// Created by wlanjie on 2018/3/16.
//

#ifndef MP4_ESDESCRIPTOR_H
#define MP4_ESDESCRIPTOR_H

#include "descriptor.h"
#include "decoder_config_descriptor.h"

namespace mp4 {

class EsDescriptor : public Descriptor {
public:
    EsDescriptor(UI16 esId);
    EsDescriptor(ByteStream& stream, Size headerSize, Size payloadSize);
    ~EsDescriptor();
    virtual Result addSubDescriptor(Descriptor* desciptor);
    virtual Result writeFields(ByteStream& stream);
    virtual const DecoderConfigDescriptor* getDecoderConfigDescription() const;
private:
    unsigned short esId;
    unsigned short ocrEsId;
    Flags flags;
    unsigned char streamPriority;
    unsigned short dependsOn;
    String url;
    mutable List<Descriptor> subDescriptors;
};

}
#endif //MP4_ESDESCRIPTOR_H
