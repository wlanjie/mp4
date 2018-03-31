//
// Created by wlanjie on 2018/3/11.
//

#include "avc_sample_description.h"
#include "utils.h"
#include "sample_entry.h"

namespace mp4 {

AvcSampleDescription::AvcSampleDescription(UI32 format,
                                           UI16 width,
                                           UI16 height,
                                           UI16 depth,
                                           const char *compressorName,
                                           Avcc *avcc) :
        SampleDescription(TYPE_AVC, format, nullptr),
        VideoSampleDescription(width, height, depth, compressorName) {
    if (avcc) {
        this->avcc = new Avcc(*avcc);
    } else {
        avcc = new Avcc();
    }
    details.addChild(avcc);
}

AvcSampleDescription::AvcSampleDescription(UI32 format,
                                           UI16 width,
                                           UI16 height,
                                           UI16 depth,
                                           const char *compressorName,
                                           AtomParent *details) :
        SampleDescription(TYPE_AVC, format, details),
        VideoSampleDescription(width, height, depth, compressorName) {
    auto* avcc = DYNAMIC_CAST(Avcc, details->getChild(ATOM_TYPE_AVCC));
    if (avcc) {
        this->avcc = avcc;
    } else {
        avcc = new Avcc();
        details->addChild(avcc);
    }
}

AvcSampleDescription::AvcSampleDescription(UI32 format,
                                           UI16 width,
                                           UI16 height,
                                           UI16 depth,
                                           const char *compressorName,
                                           UI08 profile,
                                           UI08 level,
                                           UI08 profileCompatibility,
                                           UI08 naluLenghtSize,
                                           Array<DataBuffer> &sequenceParameters,
                                           Array<DataBuffer> &pictureParameters) :
        SampleDescription(TYPE_AVC, format, nullptr),
        VideoSampleDescription(width, height, depth, compressorName) {
    avcc = new Avcc(profile, level, profileCompatibility, naluLenghtSize, sequenceParameters, pictureParameters);
    details.addChild(avcc);
}

Atom *AvcSampleDescription::toAtom() const {
    return new AvcSampleEntry(format, width, height, depth, compressorName.GetChars(), &details);
}

Result AvcSampleDescription::getCodecString(String &codec) {
    char coding[5];
    FormatFourChars(coding, getFormat());
    char workspace[64];
    FormatString(workspace, sizeof(workspace), "%s.%02X%02X%02X", coding, getProfile(), getProfileCompatibility(), getLevel());
    codec = workspace;
    return SUCCESS;
}

}
