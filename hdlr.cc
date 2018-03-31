//
// Created by wlanjie on 2018/3/9.
//

#include "hdlr.h"
#include "utils.h"

namespace mp4 {

Hdlr *Hdlr::create(Size size, ByteStream &stream) {
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
    return new Hdlr(size, version, flags, stream);
}

Hdlr::Hdlr(UI32 hdlrType, const char *hdlrName) :
        Atom(ATOM_TYPE_HDLR, FULL_ATOM_HEADER_SIZE, 0, 0),
        handlerType(hdlrType),
        handlerName(hdlrName) {
    size32 += 20 + handlerName.GetLength() + 1;
    reserved[0] = reserved[1] = reserved[2] = 0;
}

Hdlr::Hdlr(UI32 size, UI08 version, UI32 flags, ByteStream &stream) :
        Atom(ATOM_TYPE_HDLR, size, version, flags) {
    UI32 predefined;
    stream.readUI32(predefined);
    stream.readUI32(handlerType);
    stream.readUI32(reserved[0]);
    stream.readUI32(reserved[1]);
    stream.readUI32(reserved[2]);

    // read the name unless it is empty
    if (size < FULL_ATOM_HEADER_SIZE + 20) return;
    UI32 name_size = size - (FULL_ATOM_HEADER_SIZE + 20);
    auto *name = new char[name_size + 1];
    stream.read(name, name_size);
    name[name_size] = '\0'; // force a null termination
    // handle a special case: the Quicktime files have a pascal
    // string here, but ISO MP4 files have a C string.
    // we try to detect a pascal encoding and correct it.
    if ((UI08) name[0] == (UI08) (name_size - 1)) {
        handlerName = name + 1;
    } else {
        handlerName = name;
    }
    delete[] name;
}

Result Hdlr::writeFields(ByteStream &stream) {
    Result result;
    result = stream.writeUI32(0);
    if (FAILED(result)) return result;
    result = stream.writeUI32(handlerType);
    if (FAILED(result)) return result;
    result = stream.writeUI32(reserved[0]);
    if (FAILED(result)) return result;
    result = stream.writeUI32(reserved[1]);
    if (FAILED(result)) return result;
    result = stream.writeUI32(reserved[2]);
    if (FAILED(result)) return result;
    UI08 name_size = (UI08) handlerName.GetLength();
    if (FULL_ATOM_HEADER_SIZE + 20 + name_size > size32) {
        name_size = (UI08) (size32 - FULL_ATOM_HEADER_SIZE + 20);
    }
    if (name_size) {
        result = stream.write(handlerName.GetChars(), name_size);
        if (FAILED(result)) return result;
    }

    // pad with zeros if necessary
    Size padding = size32 - (FULL_ATOM_HEADER_SIZE + 20 + name_size);
    while (padding--) stream.writeUI08(0);
    return SUCCESS;
}

}
