//
// Created by wlanjie on 2018/3/10.
//

#include "sampledescription.h"
#include "atomfactory.h"
#include "sample_entry.h"
#include "utils.h"
#include "sl_config_descriptor.h"
#include "mp4_audio_info.h"

SampleDescription::SampleDescription(SampleDescription::Type type,
                                     UI32 format,
                                     AtomParent *details) :
        type(type),
        format(format) {
    if (details) {
        for (List<Atom>::Item* item = details->getChildren().FirstItem(); item; item = item->GetNext()) {
            Atom* atom = item->GetData();
            if (atom) {
                Atom* clone = atom->clone();
                if (clone) {
                    this->details.addChild(clone);
                }
            }
        }
    }
}

SampleDescription::~SampleDescription() {

}

SampleDescription *SampleDescription::clone(Result *result) {
    if (result) {
        *result = SUCCESS;
    }
    Atom* atom = toAtom();
    if (atom == nullptr) {
        if (result) {
            *result = FAILURE;
        }
        return nullptr;
    }
    MemoryByteStream* mbs = new MemoryByteStream((UI32) atom->getSize());
    atom->write(*mbs);
    delete atom;
    atom = nullptr;

    mbs->seek(0);
    AtomFactory* factory = new AtomFactory();
    factory->pushContext(ATOM_TYPE_STSD);
    Atom* atomClone = nullptr;
    auto createAtomResult = factory->createAtomFromStream(*mbs, atomClone);
    factory->popContext();
    delete factory;
    if (result) {
        *result = createAtomResult;
    }
    mbs->release();
    if (FAILED(createAtomResult)) {
        return nullptr;
    }
    SampleEntry* sampleEntry = DYNAMIC_CAST(SampleEntry, atomClone);
    if (sampleEntry == nullptr) {
        if (result) {
            *result = ERROR_INTERNAL;
            delete atomClone;
        }
        return nullptr;
    }
    SampleDescription* clone = sampleEntry->toSampleDescription();
    if (clone == nullptr) {
        if (result) {
            *result = ERROR_INTERNAL;
        }
    }
    delete atomClone;
    return clone;
}

Result SampleDescription::getCodecString(String &codec) {
    char coding[5];
    FormatFourChars(coding, format);
    codec.Assign(coding, 4);
    return SUCCESS;
}

Atom *SampleDescription::toAtom() const {
    return new SampleEntry(format);
}

Atom *GenericVideoSampleDescription::toAtom() const {
    auto* sampleEntry = new VisualSampleEntry(format, width, height, depth, compressorName.GetChars());
    AtomParent& details = const_cast<AtomParent&>(this->details);
    for (List<Atom>::Item* item = details.getChildren().FirstItem(); item; item = item->GetNext()) {
        Atom* child = item->GetData();
        sampleEntry->addChild(child->clone());
    }
    return SampleDescription::toAtom();
}

Atom *GenericAudioSampleDescription::toAtom() const {
    auto* sampleEntry = new AudioSampleEntry(format, sampleRate << 16, sampleSize, channelCount);
    AtomParent& details = const_cast<AtomParent&>(this->details);
    for (List<Atom>::Item* item = details.getChildren().FirstItem(); item; item = item->GetNext()) {
        auto* child = item->GetData();
        sampleEntry->addChild(child->clone());
    }
    return sampleEntry;
}

const char *MpegSampleDescription::getStreamTypeString(StreamType type) {
    switch (type) {
        case STREAM_TYPE_FORBIDDEN:
            return "INVALID";
        case STREAM_TYPE_OD:
            return "Object Descriptor";
        case STREAM_TYPE_CR:
            return "CR";
        case STREAM_TYPE_BIFS:
            return "BIFS";
        case STREAM_TYPE_VISUAL:
            return "Visual";
        case STREAM_TYPE_AUDIO:
            return "Audio";
        case STREAM_TYPE_MPEG7:
            return "MPEG-7";
        case STREAM_TYPE_IPMP:
            return "IPMP";
        case STREAM_TYPE_OCI:
            return "OCI";
        case STREAM_TYPE_MPEGJ:
            return "MPEG-J";
        default:
            return "UNKNOWN";
    }
}

const char *MpegSampleDescription::getObjectTypeString(OTI oti) {
    switch (oti) {
        case OTI_MPEG4_SYSTEM:         return "MPEG-4 System";
        case OTI_MPEG4_SYSTEM_COR:     return "MPEG-4 System COR";
        case OTI_MPEG4_VISUAL:         return "MPEG-4 Video";
        case OTI_MPEG4_AUDIO:          return "MPEG-4 Audio";
        case OTI_MPEG2_VISUAL_SIMPLE:  return "MPEG-2 Video Simple Profile";
        case OTI_MPEG2_VISUAL_MAIN:    return "MPEG-2 Video Main Profile";
        case OTI_MPEG2_VISUAL_SNR:     return "MPEG-2 Video SNR";
        case OTI_MPEG2_VISUAL_SPATIAL: return "MPEG-2 Video Spatial";
        case OTI_MPEG2_VISUAL_HIGH:    return "MPEG-2 Video High";
        case OTI_MPEG2_VISUAL_422:     return "MPEG-2 Video 4:2:2";
        case OTI_MPEG2_AAC_AUDIO_MAIN: return "MPEG-2 Audio AAC Main Profile";
        case OTI_MPEG2_AAC_AUDIO_LC:   return "MPEG-2 Audio AAC Low Complexity";
        case OTI_MPEG2_AAC_AUDIO_SSRP: return "MPEG-2 Audio AAC SSRP";
        case OTI_MPEG2_PART3_AUDIO:    return "MPEG-2 Audio Part-3";
        case OTI_MPEG1_VISUAL:         return "MPEG-1 Video";
        case OTI_MPEG1_AUDIO:          return "MPEG-1 Audio";
        case OTI_JPEG:                 return "JPEG";
        case OTI_JPEG2000:             return "JPEG-2000";
        case OTI_EVRC_VOICE:           return "EVRC Voice";
        case OTI_SMV_VOICE:            return "SMV Voice";
        case OTI_3GPP2_CMF:            return "3GPP2 CMF";
        case OTI_SMPTE_VC1:            return "SMPTE VC1 Video";
        case OTI_DIRAC_VIDEO:          return "Dirac Video";
        case OTI_AC3_AUDIO:            return "AC3 Audio";
        case OTI_EAC3_AUDIO:           return "E-AC3 Audio";
        case OTI_DRA_AUDIO:            return "DRA Audio";
        case OTI_G719_AUDIO:           return "G.719 Audio";
        case OTI_DTS_AUDIO:            return "DTS Audio";
        case OTI_DTS_HIRES_AUDIO:      return "DTS High Resolution Audio";
        case OTI_DTS_MASTER_AUDIO:     return "DTS Master Audio";
        case OTI_DTS_EXPRESS_AUDIO:    return "DTS Express/LBR Audio";
        case OTI_13K_VOICE:            return "13K Voice";
        default:                           return "UNKNOWN";
    }
}

MpegSampleDescription::MpegSampleDescription(UI32 format, Esds *esds) :
        SampleDescription(TYPE_MPEG, format, nullptr),
        streamType(0),
        objectTypeId(0),
        bufferSize(0),
        maxBitRate(0),
        avgBitRate(0) {
    if (esds) {
        const auto* esDesc = esds->getEsDescriptor();
        if (!esDesc) {
            return;
        }
        const auto* dcDesc = esDesc->getDecoderConfigDescription();
        if (dcDesc) {
            streamType = dcDesc->getStreamType();
            objectTypeId = dcDesc->getObjectTypeIndication();
            bufferSize = dcDesc->getBufferSize();
            maxBitRate = dcDesc->getMaxBitRate();
            avgBitRate = dcDesc->getAvgBitRate();
            const auto* dsiDesc = dcDesc->getDecoderSpecificInfoDescriptor();
            if (dsiDesc) {
                decoderInfo.setData(dsiDesc->getDecoderSpecificInfo().getData(),
                                    dsiDesc->getDecoderSpecificInfo().getDataSize());
            }
        }
    }
}

MpegSampleDescription::MpegSampleDescription(UI32 format,
                                             UI08 streamType,
                                             UI08 oti,
                                             const DataBuffer *decoderInfo,
                                             UI32 bufferSize,
                                             UI32 maxBitRate,
                                             UI32 avgBitRate) :
        SampleDescription(TYPE_MPEG, format, nullptr),
        streamType(streamType),
        objectTypeId(oti),
        bufferSize(bufferSize),
        maxBitRate(maxBitRate),
        avgBitRate(avgBitRate) {
    if (decoderInfo) {
        this->decoderInfo.setData(decoderInfo->getData(), decoderInfo->getDataSize());
    }
}

EsDescriptor *MpegSampleDescription::createEsDescriptor() const {
    auto* desc = new EsDescriptor(0);
    DecoderSpecificInfoDescriptor* dsiDesc = nullptr;
    if (decoderInfo.getDataSize() != 0) {
        dsiDesc = new DecoderSpecificInfoDescriptor(decoderInfo);
    }
    auto* decoderConfig  = new DecoderConfigDescriptor(streamType, objectTypeId, bufferSize, maxBitRate, avgBitRate, dsiDesc);
    desc->addSubDescriptor(decoderConfig);
    desc->addSubDescriptor(new SLConfigDescriptor());
    return desc;
}

const char * MpegAudioSampleDescription::getMpeg4AudioObjectTypeString(MpegAudioSampleDescription::Mpeg4AudioObjectType type) {
    switch (type) {
        case MPEG4_AUDIO_OBJECT_TYPE_AAC_MAIN:
            return "AAC Main Profile";
        case MPEG4_AUDIO_OBJECT_TYPE_AAC_LC:
            return "AAC Low Complexity";
        case MPEG4_AUDIO_OBJECT_TYPE_AAC_SSR:
            return "AAC Scalable Sample Rate";
        case MPEG4_AUDIO_OBJECT_TYPE_AAC_LTP:
            return "AAC Long Term Predictor";
        case MPEG4_AUDIO_OBJECT_TYPE_SBR:
            return "Spectral Band Replication";
        case MPEG4_AUDIO_OBJECT_TYPE_AAC_SCALABLE:
            return "AAC Scalable";
        case MPEG4_AUDIO_OBJECT_TYPE_TWINVQ:
            return "Twin VQ";
        case MPEG4_AUDIO_OBJECT_TYPE_CELP:
            return "CELP";
        case MPEG4_AUDIO_OBJECT_TYPE_HVXC:
            return "HVXC";
        case MPEG4_AUDIO_OBJECT_TYPE_TTSI:
            return "TTSI";
        case MPEG4_AUDIO_OBJECT_TYPE_MAIN_SYNTHETIC:
            return "Main Synthetic";
        case MPEG4_AUDIO_OBJECT_TYPE_WAVETABLE_SYNTHESIS:
            return "Wavetable Synthesis";
        case MPEG4_AUDIO_OBJECT_TYPE_GENERAL_MIDI:
            return "General MIDI";
        case MPEG4_AUDIO_OBJECT_TYPE_ALGORITHMIC_SYNTHESIS:
            return "Algorithmic Synthesis";
        case MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_LC:
            return "Error Resilient AAC Low Complexity";
        case MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_LTP:
            return "Error Resilient AAC Long Term Prediction";
        case MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_SCALABLE:
            return "Error Resilient AAC Scalable";
        case MPEG4_AUDIO_OBJECT_TYPE_ER_TWINVQ:
            return "Error Resilient Twin VQ";
        case MPEG4_AUDIO_OBJECT_TYPE_ER_BSAC:
            return "Error Resilient Bit Sliced Arithmetic Coding";
        case MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_LD:
            return "Error Resilient AAC Low Delay";
        case MPEG4_AUDIO_OBJECT_TYPE_ER_CELP:
            return "Error Resilient CELP";
        case MPEG4_AUDIO_OBJECT_TYPE_ER_HVXC:
            return "Error Resilient HVXC";
        case MPEG4_AUDIO_OBJECT_TYPE_ER_HILN:
            return "Error Resilient HILN";
        case MPEG4_AUDIO_OBJECT_TYPE_ER_PARAMETRIC:
            return "Error Resilient Parametric";
        case MPEG4_AUDIO_OBJECT_TYPE_SSC:
            return "SSC";
        case MPEG4_AUDIO_OBJECT_TYPE_PS:
            return "Parametric Stereo";
        case MPEG4_AUDIO_OBJECT_TYPE_MPEG_SURROUND:
            return "MPEG Surround";
        case MPEG4_AUDIO_OBJECT_TYPE_LAYER_1:
            return "MPEG Layer 1";
        case MPEG4_AUDIO_OBJECT_TYPE_LAYER_2:
            return "MPEG Layer 2";
        case MPEG4_AUDIO_OBJECT_TYPE_LAYER_3:
            return "MPEG Layer 3";
        case MPEG4_AUDIO_OBJECT_TYPE_DST:
            return "Direct Stream Transfer";
        case MPEG4_AUDIO_OBJECT_TYPE_ALS:
            return "ALS Lossless Coding";
        case MPEG4_AUDIO_OBJECT_TYPE_SLS:
            return "SLS Scalable Lossless Coding";
        case MPEG4_AUDIO_OBJECT_TYPE_SLS_NON_CORE:
            return "SLS Scalable Lossless Coding (Non Core)";
        case MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_ELD:
            return "Error Resilient AAC ELD";
        case MPEG4_AUDIO_OBJECT_TYPE_SMR_SIMPLE:
            return "SMR Simple";
        case MPEG4_AUDIO_OBJECT_TYPE_SMR_MAIN:
            return "SMR Main";
        default:
            return "UNKNOWN";
    }
}

MpegAudioSampleDescription::MpegAudioSampleDescription(UI32 sampleRate,
                                                       UI16 sampleSize,
                                                       UI16 channelCount,
                                                       Esds *esds) :
        MpegSampleDescription(ATOM_TYPE_MP4A, esds),
        AudioSampleDescription(sampleRate, sampleSize, channelCount) {

}

MpegAudioSampleDescription::MpegAudioSampleDescription(MpegSampleDescription::OTI oti,
                                                       UI32 sampleRate,
                                                       UI16 sampleSize,
                                                       UI16 channelCount,
                                                       const DataBuffer *decoderInfo,
                                                       UI32 bufferSize,
                                                       UI32 maxBitRate,
                                                       UI32 avgBitRate) :
        MpegSampleDescription(ATOM_TYPE_MP4A, STREAM_TYPE_AUDIO, oti, decoderInfo, bufferSize, maxBitRate, avgBitRate),
        AudioSampleDescription(sampleRate, sampleSize, channelCount) {

}

Result MpegAudioSampleDescription::getCodecString(String &codec) {
    char coding[5];
    FormatFourChars(coding, getFormat());
    char workspace[64];
    workspace[0] = 0;
    if (getFormat() == SAMPLE_FORMAT_MP4A) {
        if (getObjectTypeId() == OTI_MPEG4_AUDIO) {
            Mpeg4AudioObjectType object_type = getMpeg4AudioObjectType();
            if (object_type == MPEG4_AUDIO_OBJECT_TYPE_AAC_LC) {
                const DataBuffer& dsi = getDecoderInfo();
                if (dsi.getDataSize()) {
                    Mp4AudioDecoderConfig decConfig;
                    Result result = decConfig.parse(dsi.getData(), dsi.getDataSize());
                    if (SUCCEEDED(result)) {
                        if (decConfig.Extension.psPresent) {
                            object_type = MPEG4_AUDIO_OBJECT_TYPE_PS;
                        } else if (decConfig.Extension.sbrPresent) {
                            object_type = MPEG4_AUDIO_OBJECT_TYPE_SBR;
                        }
                    }
                }
            }
            FormatString(workspace, sizeof(workspace), "%s.%02X.%d", coding, (int)getObjectTypeId(), object_type);
        } else {
            FormatString(workspace, sizeof(workspace), "%s.%02X", coding, (int)getObjectTypeId());
        }
    }

    codec = workspace;
    return SUCCESS;
}

MpegAudioSampleDescription::Mpeg4AudioObjectType MpegAudioSampleDescription::getMpeg4AudioObjectType() const {
    if (objectTypeId == OTI_MPEG4_AUDIO &&
            decoderInfo.getDataSize() >= 1) {
        UI08 type = decoderInfo.getData()[0] >> 3;
        if (type == 31) {
            if (decoderInfo.getDataSize() < 2) {
                return 0;
            }
            type = 32 + (((decoderInfo.getData()[0] & 0x07) << 3) | ((decoderInfo.getData()[1] & 0xE0) >> 5));
        }
        return type;
    }
    return 0;
}

Atom *MpegAudioSampleDescription::toAtom() const {
    return new Mp4aSampleEntry(sampleRate << 16, sampleSize, channelCount, createEsDescriptor());
}
