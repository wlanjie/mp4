//
// Created by wlanjie on 2018/3/16.
//

#ifndef MP4_MP4AUDIOINFO_H
#define MP4_MP4AUDIOINFO_H

#include "types.h"
#include "databuffer.h"

namespace mp4 {

class Mp4AudioDsiParser {
public:
    Mp4AudioDsiParser(const UI08* data, Size dataSize) : data(data, dataSize), position(0) {}
    Size bitsLeft() { return 8 * data.getDataSize() - position; }
    UI32 readBits(unsigned int n) {
        UI32 result = 0;
        const UI08* data = this->data.getData();
        while (n) {
            unsigned int bitsAvail = 8 - (position % 8);
            unsigned int chunkSize = bitsAvail >= n ? n : bitsAvail;
            unsigned int chunkBits = (((unsigned int)(data[position/8]))>>(bitsAvail - chunkSize))&((1<<chunkSize)-1);
            result = (result << chunkSize) | chunkBits;
            n -= chunkSize;
            position += chunkSize;
        }
        return result;
    }

private:
    DataBuffer data;
    unsigned int position;
};

class Mp4AudioDecoderConfig {
public:
    typedef enum {
        CHANNEL_CONFIG_NONE   = 0,             /**< No channel (not used)       */
        CHANNEL_CONFIG_MONO   = 1,             /**< Mono (single audio channel) */
        CHANNEL_CONFIG_STEREO = 2,             /**< Stereo (Two audio channels) */
        CHANNEL_CONFIG_STEREO_PLUS_CENTER = 3, /**< Stereo plus one center channel */
        CHANNEL_CONFIG_STEREO_PLUS_CENTER_PLUS_REAR_MONO = 4, /**< Stereo plus one center and one read channel */
        CHANNEL_CONFIG_FIVE = 5,               /**< Five channels */
        CHANNEL_CONFIG_FIVE_PLUS_ONE = 6,      /**< Five channels plus one low frequency channel */
        CHANNEL_CONFIG_SEVEN_PLUS_ONE = 7,     /**< Seven channels plus one low frequency channel */
    } ChannelConfiguration;

    Mp4AudioDecoderConfig();
    Result parse(const UI08* data, Size dataSize);
    void reset();

    UI08 objectType;
    unsigned int samplingFrequencyIndex;
    unsigned int samplingFrequency;
    unsigned int channelCount;
    ChannelConfiguration channelConfiguration;
    bool frameLengthFlag;
    bool dependsOnCoreCoder;
    unsigned int coreCoderDelay;

    struct {
        bool sbrPresent;
        bool psPresent;
        UI08 objectType;
        unsigned int samplingFrequencyIndex;
        unsigned int samplingFrequency;
    } Extension;

private:
    Result parseAudioObjectType(Mp4AudioDsiParser& parser, UI08& objectType);
    Result parseGASpecificInfo(Mp4AudioDsiParser& parser);
    Result parseSamplingFrequency(Mp4AudioDsiParser& parser, unsigned int& samplingFrequencyIndex, unsigned int& samplingFrequency);
    Result parseExtension(Mp4AudioDsiParser& parser);
};

}
#endif //MP4_MP4AUDIOINFO_H
