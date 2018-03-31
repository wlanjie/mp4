//
// Created by wlanjie on 2018/3/7.
//

#include "ftyp.h"
#include "utils.h"

namespace mp4 {

Ftyp::Ftyp(UI32 size, ByteStream &stream) :
        Atom(ATOM_TYPE_FTYP, size),
        majorBrand(0),
        minorVersion(0) {
    if (size < 16) {
        return;
    }
    stream.readUI32(majorBrand);
    stream.readUI32(minorVersion);
    size -= 16;

    while (size >= 4) {
        UI32 compatibleBrand;
        Result result = stream.readUI32(compatibleBrand);
        if (FAILED(result)) return;
        compatibleBrands.Append(compatibleBrand);
        size -= 4;
    }
}

Ftyp::Ftyp(UI32 majorBrand, UI32 minorVersion, UI32 *compatibleBrands, Cardinal compatibleBrandCount) :
        Atom(ATOM_TYPE_FTYP, ATOM_HEADER_SIZE + 8 + 4 * compatibleBrandCount),
        majorBrand(majorBrand),
        minorVersion(minorVersion),
        compatibleBrands(compatibleBrands, compatibleBrandCount) {

}

Result Ftyp::writeFields(ByteStream &stream) {
    Result result = stream.writeUI32(majorBrand);
    if (FAILED(result)) return result;
    result = stream.writeUI32(minorVersion);
    if (FAILED(result)) return result;
    Cardinal compatBrandCount = compatibleBrands.ItemCount();
    for (Cardinal i = 0; i < compatBrandCount; i++) {
        result = stream.writeUI32(compatibleBrands[i]);
        if (FAILED(result)) return result;
    }
    return 0;
}

bool Ftyp::hasCompatibleBrand(UI32 brand) {
    for (int i = 0; i < compatibleBrands.ItemCount(); ++i) {
        if (compatibleBrands[i] == brand) {
            return true;
        }
    }
    return false;
}

}
