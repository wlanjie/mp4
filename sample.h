//
// Created by wlanjie on 2018/3/10.
//

#ifndef MP4_SAMPLE_H
#define MP4_SAMPLE_H


#include "bytestream.h"

class Sample {
public:
    Sample();
    Sample(const Sample& other);
    Sample(ByteStream& stream,
           Position offset,
           Size size,
           UI32 duration,
           Ordinal descriptionIndex,
           UI64 dts,
           UI32 ctsDelta,
           bool syncFlag);
    ~Sample();
    Sample& operator=(const Sample& other);
    Result readData(DataBuffer& data);
    Result readData(DataBuffer& data, Size size, Size offset = 0);
    void detach();

    ByteStream* getDataStream();
    void setDataStream(ByteStream& stream);
    Position getOffset() const { return offset; }
    void setOffset(Position offset) { this->offset = offset; }
    Size getSize() const { return size; }
    void setSize(Size size) { this->size = size; }
    Ordinal getDescriptionIndex() const { return descriptionIndex; }
    void setDescriptionIndex(Ordinal descriptionIndex) { this->descriptionIndex = descriptionIndex; }
    /**
     * Get the DTS (Decoding Time Stamp) of the sample in the timescale of the media
     */
    UI64 getDts() const { return dts; }
    /**
     * Set the DTS (Decoding Time Stamp) of the sample in the timescale of the media
     * @param dts
     */
    void setDts(UI64 dts) { this->dts = dts; }
    /**
     * Get the CTS (Composition Time Stamp) of the sample in the timescale of the media
     * @return
     */
    UI64 getCts() const { return dts + ctsDelta; }
    /**
     * Set the CTS (Composition Time Stamp) of the sample in the timescale of the media
     * @param cts
     */
    void setCts(UI64 cts) { ctsDelta = (cts > dts) ? (UI32) (cts - dts) : 0; }
    /**
     * Get the CTS Delta (difference between the CTS (Composition Time Stamp) and DTS (Decoding Time Stamp)
     * of the sample in the timescale of the media.
     * @return
     */
    UI32 getCtsDelta() const { return ctsDelta; }
    /**
     * Set the CTS Delta (difference between the CTS (Composition Time Stamp) and DTS (Decoding Time Stamp)
     * of the sample in the timescale of the media.*
     */
    void setCtsDelta(UI32 ctsDelta) { this->ctsDelta = ctsDelta; }
    /**
     * Get the duration of the sample in the timescale of the media
     * @return
     */
    UI32 getDuration() const { return duration; }
    /**
     * Get the duration of the sample in the timescale of the media
     * @param duration
     */
    void setDuration(UI32 duration) { this->duration = duration; }

    /**
     * Return whether the sample is a sync (random-access point) sample or not.
     * @return
     */
    bool isSync() const { return sync; }
    /**
     * Set whether the sample is a sync (random-access point) sample or not.
     * @param sync
     */
    void setSync(bool sync) { this->sync = sync; }
    /**
     * Resets the sample: will also release any data stream reference
     */
    void reset();
private:
    ByteStream* dataStream;
    Position offset;
    Size size;
    UI32 duration;
    Ordinal descriptionIndex;
    UI64 dts;
    UI32 ctsDelta;
    bool sync;
};


#endif //MP4_SAMPLE_H
