//
// Created by wlanjie on 2018/3/10.
//

#ifndef MP4_STSC_H
#define MP4_STSC_H


#include "atom.h"
#include "array.h"

class StscTableEntry {
public:
    StscTableEntry() :
            firstChunk(0),
            firstSample(0),
            chunkCount(0),
            samplesPerChunk(0),
            sampleDescriptionIndex(0) {}

    StscTableEntry(Ordinal firstChunk,
                   Ordinal firstSample,
                   Cardinal samplesPerChunk,
                   Ordinal sampleDescriptionIndex) :
            firstChunk(firstChunk),
            firstSample(firstSample),
            chunkCount(0),
            samplesPerChunk(samplesPerChunk),
            sampleDescriptionIndex(sampleDescriptionIndex) {}

    StscTableEntry(Ordinal firstChunk,
                   Ordinal firstSample,
                   Cardinal chunkCount,
                   Cardinal samplesPerChunk,
                   Ordinal sampleDescriptionIndex) :
            firstChunk(firstChunk),
            firstSample(firstSample),
            chunkCount(chunkCount),
            samplesPerChunk(samplesPerChunk),
            sampleDescriptionIndex(sampleDescriptionIndex) {}

    Ordinal firstChunk;
    Ordinal firstSample;
    Cardinal chunkCount;
    Cardinal samplesPerChunk;
    Ordinal sampleDescriptionIndex;
};

/**
 * sample-to-chunk, partial data-offset information
 */
class Stsc : public Atom {
public:
    static Stsc* create(Size size, ByteStream& stream);
    Stsc();
    virtual Result writeFields(ByteStream& stream);
    virtual Result getChunkForSample(Ordinal sample, Ordinal& chunk, Ordinal& skip, Ordinal& sampleDescriptionIndex);
    virtual Result addEntry(Cardinal chunkCount, Cardinal samplePerChunk, Ordinal sampleDescriptionIndex);

private:
    Stsc(UI32 size, UI08 version, UI32 flags, ByteStream& stream);
    Array<StscTableEntry> entries;
    Ordinal cachedChunkGroup;
};


#endif //MP4_STSC_H
