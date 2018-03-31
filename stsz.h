//
// Created by wlanjie on 2018/3/11.
//

#ifndef MP4_STSZ_H
#define MP4_STSZ_H

#include "atom.h"
#include "array.h"

namespace mp4 {

/**
 * Sample Size Boxes
 */
class Stsz : public Atom {
public:
    static Stsz* create(Size size, ByteStream& stream);
    Stsz();
    virtual Result writeFields(ByteStream& stream);
    virtual UI32 getSampleCount();
    virtual Result getSampleSize(Ordinal sample, Size& sampleSize);
    virtual Result setSampleSize(Ordinal sample, Size sampleSize);
    virtual Result addEntry(UI32 size);

private:
    Stsz(UI32 size, UI08 version, UI32 flags, ByteStream& stream);
    UI32 sampleSize;
    UI32 sampleCount;
    Array<UI32> entries;
};

}
#endif //MP4_STSZ_H
