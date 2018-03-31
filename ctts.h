//
// Created by wlanjie on 2018/3/11.
//

#ifndef MP4_CTTS_H
#define MP4_CTTS_H

#include "atom.h"
#include "array.h"

namespace mp4 {

class CttsTableEntry {
public:
    CttsTableEntry() : sampleCount(0), sampleOffset(0) {}
    CttsTableEntry(UI32 sampleCount, UI32 sampleOffset) :
            sampleCount(sampleCount), sampleOffset(sampleOffset) {}
    UI32 sampleCount;
    UI32 sampleOffset;
};

/**
 * composition time to sample box
 * http://blog.csdn.net/w839687571/article/details/41725811
 */
class Ctts : public Atom {
public:
    static Ctts* create(Size size, ByteStream& stream);
    Ctts();
    virtual Result writeFields(ByteStream& stream);
    Result addEntry(UI32 count, UI32 ctsOffset);
    Result getCtsOffset(Ordinal sample, UI32& ctsOffset);
private:
    Ctts(UI32 size, UI08 version, UI32 flags, ByteStream& stream);
    Array<CttsTableEntry> entries;
    struct {
        Ordinal sample;
        Ordinal entryIndex;
    } LookupCache;
};

}
#endif //MP4_CTTS_H
