//
// Created by wlanjie on 2018/3/10.
//

#ifndef MP4_STTS_H
#define MP4_STTS_H

#include "atom.h"
#include "array.h"

class SttsTableEntry {
public:
    SttsTableEntry() :
            sampleCount(0),
            sampleDuration(0) {}
    SttsTableEntry(UI32 sampleCount, UI32 sampleDuration) :
            sampleCount(sampleCount),
            sampleDuration(sampleDuration){}
    UI32 sampleCount;
    UI32 sampleDuration;
};

/**
 * (decoding) time-to-sample
 */
class Stts : public Atom {
public:
    static Stts* create(Size size, ByteStream& stream);
    Stts();
    virtual Result writeFields(ByteStream& stream);
    virtual Result getDts(Ordinal sample, UI64& dts, UI32* duration = nullptr);
    virtual Result addEntry(UI32 sampleCount, UI32 sampleDuration);
    virtual Result getSampleIndexForTimeStamp(UI64 ts, Ordinal& sampleIndex);

private:
    Stts(UI32 size, UI08 version, UI32 flags, ByteStream& stream);
    Array<SttsTableEntry> entries;
    struct {
        Ordinal entryIndex;
        Ordinal sample;
        UI64 dts;
    } LookupCache;
};


#endif //MP4_STTS_H
