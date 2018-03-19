//
// Created by wlanjie on 2018/3/9.
//

#include "smhd.h"

Smhd *Smhd::create(Size size, ByteStream &stream) {
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
    return new Smhd(size, version, flags, stream);
}

Smhd::Smhd(UI16 balance) :
        Atom(ATOM_TYPE_SMHD, FULL_ATOM_HEADER_SIZE + 4, 0, 0),
        balance(balance) {
    reserved = 0;
}

Smhd::Smhd(UI32 size, UI08 version, UI32 flags, ByteStream &stream) :
        Atom(ATOM_TYPE_SMHD, size, version, flags) {
    stream.readUI16(balance);
    stream.readUI16(reserved);
}

Result Smhd::writeFields(ByteStream &stream) {
    Result result = stream.writeUI16(balance);
    if (FAILED(result)) return result;
    result = stream.writeUI16(reserved);
    if (FAILED(result)) return result;
    return SUCCESS;
}