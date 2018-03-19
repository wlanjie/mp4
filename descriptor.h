//
// Created by wlanjie on 2018/3/15.
//

#ifndef MP4_DESCIPTOR_H
#define MP4_DESCIPTOR_H


#include "expandable.h"

const UI08 DESCRIPTOR_TAG_ES        = 0x03;
const UI08 DESCRIPTOR_TAG_ES_ID_INC = 0x0E;
const UI08 DESCRIPTOR_TAG_ES_ID_REF = 0x0F;

const int ES_DESCRIPTOR_FLAG_STREAM_DEPENDENCY = 1;
const int ES_DESCRIPTOR_FLAG_URL               = 2;
const int ES_DESCRIPTOR_FLAG_OCR_STREAM        = 4;

class Descriptor : public Expandable {
public:
    Descriptor(UI08 tag, Size headerSize, Size payloadSize) :
            Expandable(tag, CLASS_ID_SIZE_08, headerSize, payloadSize) {}

    UI08 getTag() {
        return (UI08) classId;
    }
};

class DescriptorListWriter : public List<Descriptor>::Item::Operator {
public:
    DescriptorListWriter(ByteStream& stream) : stream(stream) {}

    Result action(Descriptor* descriptor) const {
        return descriptor->write(stream);
    }
private:
    ByteStream& stream;
};

class DescriptorFinder : public List<Descriptor>::Item::Finder {
public:
    DescriptorFinder(UI08 tag) : tag(tag) {}
    Result Test(Descriptor* descriptor) const {
        return descriptor->getTag() == tag ? SUCCESS : FAILURE;
    }

private:
    UI08 tag;
};

#endif //MP4_DESCIPTOR_H
