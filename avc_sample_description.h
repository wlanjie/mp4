//
// Created by wlanjie on 2018/3/11.
//

#ifndef MP4_AVCSAMPLEDESCRIPTION_H
#define MP4_AVCSAMPLEDESCRIPTION_H

#include "sampledescription.h"
#include "avcc.h"

namespace mp4 {

class AvcSampleDescription : public SampleDescription,
                             public VideoSampleDescription {
public:
    AvcSampleDescription(UI32 format,
                         UI16 width,
                         UI16 height,
                         UI16 depth,
                         const char* compressorName,
                         Avcc* avcc);

    AvcSampleDescription(UI32 format,
                         UI16 width,
                         UI16 height,
                         UI16 depth,
                         const char* compressorName,
                         AtomParent* details);

    AvcSampleDescription(UI32 format,
                         UI16 width,
                         UI16 height,
                         UI16 depth,
                         const char* compressorName,
                         UI08 profile,
                         UI08 level,
                         UI08 profileCompatibility,
                         UI08 naluLenghtSize,
                         Array<DataBuffer>& sequenceParameters,
                         Array<DataBuffer>& pictureParameters);
    UI08 getConfigurationVersion() const { return avcc->getConfigurationVersion(); }
    UI08 getProfile() const { return avcc->getProfile(); }
    UI08 getLevel() const { return avcc->getLevel(); }
    UI08 getProfileCompatibility() const { return avcc->getProfileCompatibility(); }
    UI08 getNaluLength() const { return avcc->getNaluLengthSize(); }
    Array<DataBuffer>& getSequenceParameters() { return avcc->getSequenceParameters(); }
    Array<DataBuffer>& getPictureParameters() { return avcc->getPictureParameters(); }
    const DataBuffer& getRawBytes() const { return avcc->getRawBytes(); }

    virtual Atom* toAtom() const;
    virtual Result getCodecString(String& codec);

    static const char* getProfileName(UI08 profile) {
        return Avcc::getProfileName(profile);
    }
private:
    Avcc* avcc;
};

}
#endif //MP4_AVCSAMPLEDESCRIPTION_H
