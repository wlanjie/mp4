//
// Created by wlanjie on 2018/3/15.
//

#ifndef MP4_ADTSPARSER_H
#define MP4_ADTSPARSER_H

#include "types.h"
#include "bit_stream.h"

class AdtsHeader {
public:
    AdtsHeader(const UI08* bytes);
    Result check();

    unsigned int id;
    unsigned int protectionAbsent;
    unsigned int profileObjectType;
    unsigned int samplingFrequencyIndex;
    unsigned channelConfiguration;

    unsigned int frameLength;
    unsigned int rawDataBlocks;

    static bool matchFixed(unsigned char* a, unsigned char* b);
};

typedef enum {
    AAC_STANDARD_MPEG2,
    AAC_STANDARD_MPEG4
} AacStandard;

typedef enum {
    AAC_PROFILE_MAIN,
    AAC_PROFILE_LC,
    AAC_PROFILE_SSR,
    AAC_PROFILE_LTP,
} AacProfile;

typedef struct {
    AacStandard standard;
    AacProfile profile;
    unsigned int samplingFrequencyIndex;
    UI32 samplingFrequency;
    unsigned int channelConfiguration;
    unsigned frameLength;
} AacFrameInfo;

typedef struct {
    BitStream* source;
    AacFrameInfo info;
} AacFrame;

class AdtsParser {
public:
    AdtsParser();
    ~AdtsParser();

    Result reset();
    Result feed(const UI08* buffer, Size* bufferSize, Flags flags = 0);
    Result findFrame(AacFrame& frame);
    Result skip(Size size);
    Size getBytesFree();
    Size getBytesAvailable();

private:
    Result findHeader(UI08* header);
    BitStream bits;
    Cardinal frameCount;
};


#endif //MP4_ADTSPARSER_H
