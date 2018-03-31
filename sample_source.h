//
// Created by wlanjie on 2018/3/31.
//

#ifndef MP4_SAMPLE_SOURCE_H
#define MP4_SAMPLE_SOURCE_H

#include "types.h"
#include "sample.h"
#include "sampledescription.h"
#include "track.h"

namespace mp4 {

class SampleSource {
public:
    SampleSource(){}
    virtual ~SampleSource() {}

    /**
     * return the timescale of the sample's media
     * @return
     */
    virtual UI32 getTimeScale() = 0;

    /**
     * return the duration in milliseconds
     * @return
     */
    virtual UI32 getDurationMs() = 0;

    /**
     * read the next sample form the source
     * @param sample
     * @param buffer
     * @param trackId
     * @return
     */
    virtual Result readNextSample(Sample& sample, DataBuffer& buffer, UI32& trackId) = 0;

    /**
     * seek to the sample closest to a specific timestamp in milliseconds
     * @param timeMs
     * @param before
     * @return
     */
    virtual Result seekToTime(UI32 timeMs, bool before = true) = 0;

    /**
     * return a sample description by index
     * @param index
     * @return NULL if there is no sample description with the requested index.
     */
    virtual SampleDescription* getSampleDescription(Ordinal index) = 0;
};

class TrackSampleSource : public SampleSource {
public:
    TrackSampleSource(Track* track);
    UI32 getTimeScale() override ;
    UI32 getDurationMs() override ;
    Result readNextSample(Sample& sample, DataBuffer& buffer, UI32& trackId) override ;
    Result seekToTime(UI32 timeMs, bool before = true);
    SampleDescription* getSampleDescription(Ordinal index) override ;
private:
    Track* track;
    Ordinal sampleIndex;
};

}
#endif //MP4_SAMPLE_SOURCE_H
