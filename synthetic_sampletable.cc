//
// Created by wlanjie on 2018/3/10.
//

#include "synthetic_sampletable.h"

namespace mp4 {

SyntheticSampleTable::SyntheticSampleTable(Cardinal chunkSize) :
        chunkSize(chunkSize ? chunkSize : SYNTHETIC_SAMPLE_TABLE_DEFAULT_CHUNK_SIZE) {
    LookupCache.sample = 0;
    LookupCache.chunk = 0;
}

SyntheticSampleTable::~SyntheticSampleTable() {
    sampleDescriptions.DeleteReferences();
}

Result SyntheticSampleTable::getSample(Ordinal index, Sample &sample) {
    if (index >= samples.ItemCount()) {
        return ERROR_OUT_OF_RANGE;
    }
    sample = samples[index];
    return SUCCESS;
}

Cardinal SyntheticSampleTable::getSampleCount() {
    return samples.ItemCount();
}

Result
SyntheticSampleTable::getSampleChunkPosition(Ordinal sampleIndex, Ordinal &chunkIndex, Ordinal &positionInChunk) {
    chunkIndex = 0;
    positionInChunk = 0;
    if (sampleIndex >= samples.ItemCount()) {
        return ERROR_OUT_OF_RANGE;
    }
    Ordinal sampleCursor = 0;
    Ordinal chunkCursor = 0;
    if (sampleIndex >= LookupCache.sample) {
        sampleCursor = LookupCache.sample;
        chunkCursor = LookupCache.chunk;
    }
    for (; chunkCursor < sampleInChunk.ItemCount(); sampleCursor += sampleInChunk[chunkCursor++]) {
        if (sampleCursor + sampleInChunk[chunkCursor] > sampleIndex) {
            chunkIndex = chunkCursor;
            positionInChunk = sampleIndex - sampleCursor;
            LookupCache.sample = sampleCursor;
            LookupCache.chunk = chunkCursor;
            return SUCCESS;
        }
    }
    return ERROR_OUT_OF_RANGE;
}

Cardinal SyntheticSampleTable::getSampleDescriptionCount() {
    return sampleDescriptions.ItemCount();
}

SampleDescription *SyntheticSampleTable::getSampleDescription(Ordinal index) {
    SampleDescriptionHolder* holder;
    return (SUCCEEDED(sampleDescriptions.Get(index, holder))) ? holder->sampleDescription : nullptr;
}

Result SyntheticSampleTable::getSampleIndexForTimeStamp(UI64 ts, Ordinal &index) {
    return ERROR_NOT_SUPPORTED;
}

Ordinal SyntheticSampleTable::getNearestSyncSampleIndex(Ordinal index, bool before) {
    if (before) {
        for (int i = index; i > 0; i--) {
            if (samples[i].isSync()) {
                return i;
            }
        }
        return 0;
    } else {
        Cardinal entryCount = samples.ItemCount();
        for (unsigned int i = index; i < entryCount; i++) {
            if (samples[i].isSync()) {
                return i;
            }
        }
        return samples.ItemCount();
    }
}

Result SyntheticSampleTable::addSampleDescription(SampleDescription *description, bool transferOwnership) {
    return sampleDescriptions.Add(new SampleDescriptionHolder(description, transferOwnership));
}

Result SyntheticSampleTable::addSample(ByteStream &stream, Position offset, Size size, UI32 duration,
                                        Ordinal descriptionIndex, UI64 dts, UI32 ctsDelta, bool sync) {
    if (sampleInChunk.ItemCount() == 0 ||
            sampleInChunk[sampleInChunk.ItemCount() - 1] >= chunkSize ||
            samples.ItemCount() == 0 ||
            samples[samples.ItemCount() - 1].getDescriptionIndex() != descriptionIndex) {
        sampleInChunk.Append(1);
    } else {
        ++sampleInChunk[sampleInChunk.ItemCount() - 1];
    }
    if (samples.ItemCount() > 0) {
        Sample* prevSample = &samples[samples.ItemCount() - 1];
        if (dts == 0) {
            if (prevSample->getDuration() == 0) {
                return ERROR_INVALID_PARAMETERS;
            }
            dts = prevSample->getDts() + prevSample->getDuration();
        } else {
            if (prevSample->getDuration() == 0) {
                if (dts <= prevSample->getDts()) {
                    return ERROR_INVALID_PARAMETERS;
                }
                prevSample->setDuration((UI32) (dts - prevSample->getDts()));
            } else {
                if (dts != prevSample->getDts() + prevSample->getDuration()) {
                    return ERROR_INVALID_PARAMETERS;
                }
            }
        }
    }
    Sample sample(stream, offset, size, duration, descriptionIndex, dts, ctsDelta, sync);
    return samples.Append(sample);
}

Result SyntheticSampleTable::addSample(const Sample &sample) {
    return samples.Append(sample);
}

}
