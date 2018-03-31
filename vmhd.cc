//
// Created by wlanjie on 2018/3/9.
//

#include "vmhd.h"
#include "utils.h"

namespace mp4 {

Vmhd *Vmhd::create(Size size, ByteStream &stream) {
    UI08 version;
    UI32 flags;
    if (size < FULL_ATOM_HEADER_SIZE) return nullptr;
    if (FAILED(Atom::readFullHeader(stream, version, flags))) return nullptr;
    if (version != 0) return nullptr;
    return new Vmhd(size, version, flags, stream);
}

Vmhd::Vmhd(UI16 graphicsMode, UI16 r, UI16 g, UI16 b) :
        Atom(ATOM_TYPE_VMHD, FULL_ATOM_HEADER_SIZE + 8, 0, 1),
        graphicsMode(graphicsMode) {
    opColor[0] = r;
    opColor[1] = g;
    opColor[2] = b;
}

Vmhd::Vmhd(UI32 size, UI08 version, UI32 flags, ByteStream &stream) :
        Atom(ATOM_TYPE_VMHD, size, version, flags) {
    stream.readUI16(graphicsMode);
    stream.read(opColor, sizeof(opColor));
}

Result Vmhd::writeFields(ByteStream &stream) {
    Result  result = stream.writeUI16(graphicsMode);
    if (FAILED(result)) return result;
    result = stream.write(opColor, sizeof(opColor));
    if (FAILED(result)) return result;
    return SUCCESS;
}

}
