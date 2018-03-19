//
// Created by wlanjie on 2018/3/15.
//

#include "esds.h"
#include "descriptor_factory.h"

Esds *Esds::create(Size size, ByteStream &stream) {
    UI08 version;
    UI32 flags;
    if (size < FULL_ATOM_HEADER_SIZE) {
        return nullptr;
    }
    if (FAILED(Atom::readFullHeader(stream, version, flags))) {
        return nullptr;
    }
    if (version != 0) {
        return nullptr;
    }
    return new Esds(size, version, flags, stream);
}

Esds::Esds(EsDescriptor *descriptor) :
        Atom(ATOM_TYPE_ESDS, FULL_ATOM_HEADER_SIZE, 0, 0),
        descriptor(descriptor) {
    if (descriptor) {
        size32 += descriptor->getSize();
    }
}

Esds::Esds(UI32 size, UI08 version, UI32 flags, ByteStream &stream) :
        Atom(ATOM_TYPE_ESDS, size, version, flags) {
    Descriptor* descriptor = nullptr;
    if (DescriptorFactory::createDescriptionFromStream(stream, descriptor) == SUCCESS) {
        descriptor = DYNAMIC_CAST(EsDescriptor, descriptor);
    } else {
        descriptor = nullptr;
    }
}

Esds::~Esds() {
    delete descriptor;
}

Result Esds::writeFields(ByteStream &stream) {
    if (descriptor) {
        return descriptor->write(stream);
    }
    return SUCCESS;
}