//
// Created by wlanjie on 2018/3/10.
//

#ifndef MP4_SYNCTHETICSAMPLETABLE_H
#define MP4_SYNCTHETICSAMPLETABLE_H

#include "sampletable.h"
#include "sample.h"

namespace mp4 {

const Cardinal SYNTHETIC_SAMPLE_TABLE_DEFAULT_CHUNK_SIZE = 10;

class SyntheticSampleTable : public SampleTable {
public:
    SyntheticSampleTable(Cardinal chunkSize = SYNTHETIC_SAMPLE_TABLE_DEFAULT_CHUNK_SIZE);
    virtual ~SyntheticSampleTable();
    virtual Result getSample(Ordinal index, Sample& sample);
    virtual Cardinal getSampleCount();
    virtual Result getSampleChunkPosition(Ordinal sampleIndex, Ordinal& chunkIndex, Ordinal& positionInChunk);
    virtual Cardinal getSampleDescriptionCount();
    virtual SampleDescription* getSampleDescription(Ordinal index);
    virtual Result getSampleIndexForTimeStamp(UI64 ts, Ordinal& index);
    virtual Ordinal getNearestSyncSampleIndex(Ordinal index, bool before);
    /**
     * Add a sample description to the sample table.
     * Each added sample description will have the next available index, starting at 0
     *
     * @param description Pointer to the sample description to add
     * @param transfer_ownership Boolean flag indicating whether the ownership of the
     * sample description object is transfered to the sample table object (true by default).
     * If true, the sample table object will own the sample description object, and will
     * delete it when it is itself deleted. If false, the ownership remains with the caller,
     * and only a referencing pointer is kept, thus the caller must ensure that the object
     * is not deleted before the sample table is deleted.
     */
    virtual Result addSampleDescription(SampleDescription* description, bool transferOwnership = true) ;
    /**
    * Add a sample to the sample table, where the sample duration is given
    *
    * @param data_stream The byte stream that contains the sample data. The sample
    * object added to the track will keep a reference to that byte stream.
    * @param offset Position of the first byte of sample data within the stream
    * @param size Size in bytes of the sample data
    * @param duration Duration of the sample (in the timescale of the media). This
    * value can be 0 if the duration is not known. In that case, the next sample
    * added to the table MUST have a non-zero value for the DTS (decoding timestamp),
    * which will allow the actual duration of this sample to be computed.
    * @param description_index Index of the sample description that applies to
    * this sample (typically 0).
    * @param dts DTS (decoding timestamp) of the sample. If this value is 0, and there
    * already are samples in the table, the DTS of the sample will be automatically
    * computed based on the DTS and duration of the preceding sample. If this value is
    * not equal to the DTS+duration of the preceding sample, the duration of the
    * preceding sample is automatically adjusted, unless it has a non-zero value, in which
    * case AP4_ERROR_INVALID_PARAMETERS is returned.
    * The DTS of the first sample in the table MUST always be 0.
    * @param cts_delta Difference between the CTS (composition/display timestamp) and DTS
    * (decoding timestamp) of the sample (in the timescale of the media)
    * @param sync Boolean flag indicating whether this is a sync sample or not.
    */
    virtual Result addSample(ByteStream& stream, Position offset, Size size, UI32 duration, Ordinal descriptionIndex, UI64 dts, UI32 ctsDelta, bool sync);
    virtual Result addSample(const Sample& sample);
    Sample& useSample(Ordinal index) { return samples[index]; }

private:
    class SampleDescriptionHolder {
    public:
        SampleDescriptionHolder(SampleDescription* description, bool isOwned) :
                sampleDescription(description),
                isOwned(isOwned){}
        ~SampleDescriptionHolder() {
            if (isOwned) {
                delete sampleDescription;
            }
        }
        SampleDescription* sampleDescription;
        bool isOwned;
    };
    Array<Sample> samples;
    List<SampleDescriptionHolder> sampleDescriptions;
    Cardinal chunkSize;
    Array<UI32> sampleInChunk;
    struct {
        Ordinal sample;
        Ordinal chunk;
    } LookupCache;
};

}
#endif //MP4_SYNCTHETICSAMPLETABLE_H
