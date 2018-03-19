//
// Created by wlanjie on 2018/3/10.
//

#ifndef MP4_SAMPLETABLE_H
#define MP4_SAMPLETABLE_H


#include "types.h"
#include "container.h"
#include "sampledescription.h"
#include "sample.h"
#include "stsc.h"
#include "stco.h"
#include "stsz.h"
#include "stts.h"
#include "ctts.h"
#include "stsd.h"
#include "stss.h"
#include "co64.h"

class SampleTable {
public:
    virtual ~SampleTable(){};
    virtual Result generateStbl(Container*& stbl);
    virtual Cardinal getSampleCount() = 0;
    virtual Result getSample(Ordinal sampleIndex, Sample& sample)= 0;
    virtual Result getSampleChunkPosition(Ordinal sampleIndex, Ordinal& chunkIndex, Ordinal& positionInChunk) = 0;
    virtual Cardinal getSampleDescriptionCount() = 0;
    virtual SampleDescription* getSampleDescription(Ordinal index) = 0;
    virtual Result getSampleIndexForTimeStamp(UI64 ts, Ordinal& index) = 0;
    virtual Ordinal getNearestSyncSampleIndex(Ordinal index, bool before = true) = 0;
};


#endif //MP4_SAMPLETABLE_H
