//
// Created by wlanjie on 2018/3/7.
//

#ifndef MP4_FTYPATOM_H
#define MP4_FTYPATOM_H

#include "atom.h"
#include "array.h"

namespace mp4 {

class Ftyp : public Atom {
public:

    static Ftyp* create(Size size, ByteStream& stream) {
        return new Ftyp(size, stream);
    }

    Ftyp(UI32 majorBran, UI32 minorVersion, UI32* compatibleBrands = NULL, Cardinal compatibleBrandCount = 0);
    virtual Result writeFields(ByteStream& stream);

    UI32 getMajorBrand() { return majorBrand; }
    UI32 getMinorVersion() { return minorVersion; }
    Array<UI32> getCompatibleBrands() { return compatibleBrands; }
    bool hasCompatibleBrand(UI32 brand);
    void setMajorBrandAndVersion(UI32 majorBrand, UI32 minorVersion) {
        this->majorBrand = majorBrand;
        this->minorVersion = minorVersion;
    }
private:
    Ftyp(UI32 size, ByteStream& stream);

    UI32 majorBrand;
    UI32 minorVersion;
    Array<UI32> compatibleBrands;
};

}
#endif //MP4_FTYPATOM_H
