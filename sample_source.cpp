//
// Created by wlanjie on 2018/3/31.
//

#include "sample_source.h"

namespace mp4 {

TrackSampleSource::TrackSampleSource(Track *track) :
        track(track),
        sampleIndex(0) {

}

UI32 TrackSampleSource::getTimeScale() {
    return track->getMediaTimeScale();
}

UI32 TrackSampleSource::getDurationMs() {
    return track->getDurationMs();
}

Result TrackSampleSource::readNextSample(Sample &sample, DataBuffer &buffer, UI32 &trackId) {
    auto result = track->readSample(sampleIndex, sample, buffer);
    if (SUCCEEDED(result)) {
        ++sampleIndex;
        trackId = track->getId();
    } else {
        trackId = 0;
    }
    return result;
}

Result TrackSampleSource::seekToTime(UI32 timeMs, bool before) {
    Ordinal sampleIndex = 0;
    auto result = track->getSampleIndexForTimeStampMs(timeMs, sampleIndex);
    if (FAILED(result)) {
        return result;
    }
    if (sampleIndex >= track->getSampleCount()) {
        return ERROR_OUT_OF_RANGE;
    }
    sampleIndex = track->getNearestSyncSampleIndex(sampleIndex, before);
    if (sampleIndex >= track->getSampleCount()) {
        return ERROR_OUT_OF_RANGE;
    }
    this->sampleIndex = sampleIndex;
    return result;
}

SampleDescription *TrackSampleSource::getSampleDescription(Ordinal index) {
    return track->getSampleDescription(index);
}

}
