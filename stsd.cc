//
// Created by wlanjie on 2018/3/10.
//

#include "stsd.h"

namespace mp4 {

Stsd *Stsd::create(Size size, ByteStream &stream, AtomFactory &factory) {
    UI08 version;
    UI32 flags;
    if (size < FULL_ATOM_HEADER_SIZE) {
        return nullptr;
    }
    if (FAILED(Atom::readFullHeader(stream, version, flags))) {
        return nullptr;
    }
    if (version > 1) {
        return nullptr;
    }
    return new Stsd(size, version, flags, stream, factory);
}

Stsd::Stsd(SampleTable *sampleTable) : Container(ATOM_TYPE_STSD, (UI32) 0, (UI32) 0) {
    size32 += 4;
    Cardinal sampleDescriptionCount = sampleTable->getSampleDescriptionCount();
    sampleDescriptions.EnsureCapacity(sampleDescriptionCount);
    for (Ordinal i = 0; i < sampleDescriptionCount; i++) {
        sampleDescriptions.Append(nullptr);

        SampleDescription* sampleDescription = sampleTable->getSampleDescription(i);
        Atom* entry = sampleDescription->toAtom();
        addChild(entry);
    }
}

Stsd::Stsd(UI32 size, UI08 version, UI32 flags, ByteStream &stream, AtomFactory &factory) :
        Container(ATOM_TYPE_STSD, size, false, version, flags) {
    UI32 entryCount;
    stream.readUI32(entryCount);
    factory.pushContext(type);
    LargeSize bytesAvailable = size - FULL_ATOM_HEADER_SIZE - 4;
    for (unsigned int i = 0; i < entryCount; i++) {
        Atom* atom;
        if (SUCCEEDED(factory.createAtomFromStream(stream, bytesAvailable, atom))) {
            atom->setParent(this);
            children.Add(atom);
        }
    }
    factory.popContext();
    sampleDescriptions.EnsureCapacity(children.ItemCount());
    for (Ordinal i = 0; i < children.ItemCount(); i++) {
        sampleDescriptions.Append(nullptr);
    }
}

Stsd::~Stsd() {
    for (Ordinal i = 0; i < sampleDescriptions.ItemCount(); i++) {
        delete sampleDescriptions[i];
    }
}

Result Stsd::writeFields(ByteStream &stream) {
    Result result = stream.writeUI32(children.ItemCount());
    if (FAILED(result)) {
        return result;
    }
    return children.Apply(AtomListWriter(stream));
}

Cardinal Stsd::getSampleDescriptionCount() {
    return children.ItemCount();
}

SampleDescription *Stsd::getSampleDescription(Ordinal index) {
    if (index >= children.ItemCount()) {
        return nullptr;
    }
    if (sampleDescriptions[index]) {
        return sampleDescriptions[index];
    }
    Atom* entry;
    children.Get(index, entry);
    SampleEntry* sampleEntry = DYNAMIC_CAST(SampleEntry, entry);
    if (sampleEntry == nullptr) {
//        sampleDescriptions[index] = new
        //TODO unknow sample description
    } else {
        sampleDescriptions[index] = sampleEntry->toSampleDescription();
    }
    return sampleDescriptions[index];
}

SampleEntry *Stsd::getSampleEntry(Ordinal index) {
    if (index >- children.ItemCount()) {
        return nullptr;
    }
    Atom* entry;
    children.Get(index, entry);
    return DYNAMIC_CAST(SampleEntry, entry);
}

void Stsd::onChildChanged(Atom *child) {
    UI64 size = getHeaderSize() + 4;
    children.Apply(AtomSizeAdder(size));
    size32 = (UI32) size;
    if (parent) {
        parent->onChildChanged(child);
    }
}

}
