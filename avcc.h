//
// Created by wlanjie on 2018/3/11.
//

#ifndef MP4_AVCC_H
#define MP4_AVCC_H

#include "atom.h"
#include "array.h"

namespace mp4 {

const UI08 AVC_PROFILE_BASELINE = 66;
const UI08 AVC_PROFILE_MAIN     = 77;
const UI08 AVC_PROFILE_EXTENDED = 88;
const UI08 AVC_PROFILE_HIGH     = 100;
const UI08 AVC_PROFILE_HIGH_10  = 110;
const UI08 AVC_PROFILE_HIGH_422 = 122;
const UI08 AVC_PROFILE_HIGH_444 = 144;

class Avcc : public Atom {
public:
    static Avcc* create(Size size, ByteStream& stream);
    static const char* getProfileName(UI08 profile);
    Avcc();
    Avcc(UI08 profile,
         UI08 level,
         UI08 profileCompatibility,
         UI08 lengthSize,
         Array<DataBuffer>& sequenceParameters,
         Array<DataBuffer>& pictureParameters);
    Avcc(const Avcc& other);
    virtual Result writeFields(ByteStream& stream);

    UI08 getConfigurationVersion() const { return configurationVersion; }
    UI08 getProfile() const { return profile; }
    UI08 getLevel() const { return level; }
    UI08 getProfileCompatibility() const { return profileCompatibility; }
    UI08 getNaluLengthSize() const { return naluLengthSize; }
    Array<DataBuffer>& getSequenceParameters() { return sequenceParameters; }
    Array<DataBuffer>& getPictureParameters() { return pictureParamters; }
    const DataBuffer& getRawBytes() const { return rawBytes; }
private:
    Avcc(UI32 size, const UI08* payload);
    void updateRawBytes();
    UI08 configurationVersion;
    UI08 profile;
    UI08 level;
    UI08 profileCompatibility;
    UI08 naluLengthSize;
    Array<DataBuffer> sequenceParameters;
    Array<DataBuffer> pictureParamters;
    DataBuffer rawBytes;
};

}
#endif //MP4_AVCC_H
