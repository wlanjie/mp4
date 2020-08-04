//
// Created by wlanjie on 2018/3/11.
//

#ifndef MP4_ATOMSAMPLETABLE_H
#define MP4_ATOMSAMPLETABLE_H


#include "sampletable.h"
#include "stz2.h"

namespace mp4 {

class AtomSampleTable : public SampleTable {
public:
    AtomSampleTable(Container* stbl, ByteStream& stream);
    virtual ~AtomSampleTable();
    virtual Result getSample(Ordinal sampleIndex, Sample& sample);
    virtual Cardinal getSampleCount();
    virtual SampleDescription* getSampleDescription(Ordinal index);
    virtual Cardinal getSampleDescriptionCount();
    virtual Result getSampleChunkPosition(Ordinal sampleIndex, Ordinal& chunkIndex, Ordinal& positionInChunk);
    virtual Result getSampleIndexForTimeStamp(UI64 ts, Ordinal& sampleIndex);
    virtual Result getSampleTimeStamp(Ordinal sampleIndex, UI64& ts);
    virtual Ordinal getNearestSyncSampleIndex(Ordinal index, bool before = true);
    virtual Result getChunkForSample(Ordinal sampleIndex, Ordinal& chunkIndex, Ordinal& positionInChunk, Ordinal& sampleDescriptionIndex);
    virtual Result getChunkOffset(Ordinal chunkIndex, Position& offset);
    virtual Result setChunkOffset(Ordinal chunkIndex, Position offset);
    virtual Result setSampleSize(Ordinal sampleIndex, Size size);
private:
    ByteStream& stream;
    Stsc* stsc;
    Stco* stco;
    Stsz* stsz;
    Stz2* stz2;
    Stts* stts;
    Ctts* ctts;
    Stsd* stsd;
    Stss* stss;
    Co64* co64;
};

}

#endif //MP4_ATOMSAMPLETABLE_H
