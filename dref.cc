//
// Created by wlanjie on 2018/3/9.
//

#include "dref.h"

Dref *Dref::create(Size size, ByteStream &stream, AtomFactory &factory) {
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
    return new Dref(size, version, flags, stream, factory);
}

Dref::Dref(Atom **refs, Cardinal refsCount) :
        Container(ATOM_TYPE_DREF, (UI32) 0, (UI32) 0) {
    size32 += 4;
    for (unsigned int i = 0; i < refsCount; ++i) {
        children.Add(refs[i]);
        size32 += (UI32) refs[i]->getSize();
    }
}

Dref::Dref(UI32 size,
           UI08 version,
           UI32 flags,
           ByteStream &stream,
           AtomFactory& factory) : Container(ATOM_TYPE_DREF, size, false, version, flags) {
    UI32 entryCount;
    stream.readUI32(entryCount);
    LargeSize bytesAvailable = size - FULL_ATOM_HEADER_SIZE - 4;
    while (entryCount--) {
        Atom* atom;
        while (SUCCEEDED(factory.createAtomFromStream(stream, bytesAvailable, atom))) {
            children.Add(atom);
        }
    }
}

Result Dref::writeFields(ByteStream &stream) {
    Result result = stream.writeUI32(children.ItemCount());
    if (FAILED(result)) return result;
    return children.Apply(AtomListWriter(stream));
}