//
// Created by wlanjie on 2018/3/10.
//

#ifndef MP4_SAMPLEDESCRIPTION_H
#define MP4_SAMPLEDESCRIPTION_H

#include "types.h"
#include "atom.h"
#include "esds.h"

namespace mp4 {

const UI32 SAMPLE_FORMAT_AVC1 = ATOM_TYPE('a','v','c','1');
const UI32 SAMPLE_FORMAT_AVC2 = ATOM_TYPE('a','v','c','2');
const UI32 SAMPLE_FORMAT_AVC3 = ATOM_TYPE('a','v','c','3');
const UI32 SAMPLE_FORMAT_AVC4 = ATOM_TYPE('a','v','c','4');
const UI32 SAMPLE_FORMAT_DVAV = ATOM_TYPE('d','v','a','v');
const UI32 SAMPLE_FORMAT_MP4A = ATOM_TYPE('m','p','4','a');

class SampleDescription {
public:
    enum Type {
        TYPE_UNKNOWN = 0x00,
        TYPE_MPEG = 0x01,
        TYPE_PROTECTED = 0x02,
        TYPE_AVC = 0x03,
        TYPE_HEVC = 0x04
    };

    SampleDescription(Type type, UI32 format, AtomParent* details);
    ~SampleDescription();
    virtual SampleDescription* clone(Result* result = nullptr);
    Type getType() const { return type; }
    UI32 getFormat() const { return format; }
    const AtomParent& getDetails() const { return details; }
    virtual Result getCodecString(String& codec);
    virtual Atom* toAtom() const;
protected:
    Type type;
    UI32 format;
    AtomParent details;
};

class VideoSampleDescription {
public:
    VideoSampleDescription(UI16 width, UI16 height, UI16 depth, const char* compressorName) :
            width(width),
            height(height),
            depth(depth),
            compressorName(compressorName) {}

    virtual ~VideoSampleDescription(){}

    UI16 getWidth() { return width; }
    UI16 getHeight() { return height; }
    UI16 getDepth() { return depth; }
    const char* getCompressorName() { return compressorName.GetChars(); }

protected:
    UI16 width;
    UI16 height;
    UI16 depth;
    String compressorName;
};

// TODO
class GenericVideoSampleDescription : public SampleDescription, public VideoSampleDescription {
public:
    GenericVideoSampleDescription(UI32 format,
                                  UI16 width,
                                  UI16 height,
                                  UI16 depth,
                                  const char* compressorName,
                                  AtomParent* details) :
            SampleDescription(TYPE_UNKNOWN, format, details),
            VideoSampleDescription(width, height, depth, compressorName) {}

    virtual Atom* toAtom() const;
};

class AudioSampleDescription {
public:
    AudioSampleDescription(UI32 sampleRate, UI16 sampleSize, UI16 channelCount) :
            sampleRate(sampleRate), sampleSize(sampleSize), channelCount(channelCount) {}

    virtual ~AudioSampleDescription() {}
    UI32 getSampleRate() { return sampleRate; }
    UI16 getSampleSize() { return sampleSize; }
    UI16 getChannelCount() { return channelCount; }

protected:
    UI32 sampleRate;
    UI16 sampleSize;
    UI16 channelCount;
};

class GenericAudioSampleDescription : public SampleDescription, public AudioSampleDescription {
public:
    GenericAudioSampleDescription(UI32 format, UI32 sampleRate, UI16 sampleSize, UI16 channelCount, AtomParent* details) :
            SampleDescription(TYPE_UNKNOWN, format, details),
            AudioSampleDescription(sampleRate, sampleSize, channelCount) {}

    virtual Atom* toAtom() const;
};

class MpegSampleDescription : public SampleDescription {
public:
    typedef UI08 StreamType;
    typedef UI08 OTI;

    static const char* getStreamTypeString(StreamType type);
    static const char* getObjectTypeString(OTI oti);
    MpegSampleDescription(UI32 format, Esds* esds);
    MpegSampleDescription(UI32 format,
                          UI08 streamType,
                          UI08 oti,
                          const DataBuffer* decoderInfo,
                          UI32 bufferSize,
                          UI32 maxBitRate,
                          UI32 avgBitRate);

    Byte getStreamType() { return streamType; }
    Byte getObjectTypeId() { return objectTypeId; }
    UI32 getBufferSize() { return bufferSize; }
    UI32 getMaxBitRate() { return maxBitRate; }
    UI32 getAvgBitRate() { return avgBitRate; }
    DataBuffer& getDecoderInfo() { return decoderInfo; }

    EsDescriptor* createEsDescriptor() const;

protected:
    UI32 format;
    StreamType streamType;
    OTI objectTypeId;
    UI32 bufferSize;
    UI32 maxBitRate;
    UI32 avgBitRate;
    DataBuffer decoderInfo;
};

class MpegAudioSampleDescription : public MpegSampleDescription, public AudioSampleDescription {
public:
    typedef UI08 Mpeg4AudioObjectType;
    static const char* getMpeg4AudioObjectTypeString(Mpeg4AudioObjectType type);
    MpegAudioSampleDescription(UI32 sampleRate, UI16 sampleSize, UI16 channelCount, Esds* esds);
    MpegAudioSampleDescription(OTI oti,
                               UI32 sampleRate,
                               UI16 sampleSize,
                               UI16 channelCount,
                               const DataBuffer* decoderInfo,
                               UI32 bufferSize,
                               UI32 maxBitRate,
                               UI32 avgBitRate);
    Result getCodecString(String& codec) override ;
    Atom* toAtom() const override;
    Mpeg4AudioObjectType getMpeg4AudioObjectType() const;
};

const MpegSampleDescription::StreamType STREAM_TYPE_FORBIDDEN = 0x00;
const MpegSampleDescription::StreamType STREAM_TYPE_OD        = 0x01;
const MpegSampleDescription::StreamType STREAM_TYPE_CR        = 0x02;
const MpegSampleDescription::StreamType STREAM_TYPE_BIFS      = 0x03;
const MpegSampleDescription::StreamType STREAM_TYPE_VISUAL    = 0x04;
const MpegSampleDescription::StreamType STREAM_TYPE_AUDIO     = 0x05;
const MpegSampleDescription::StreamType STREAM_TYPE_MPEG7     = 0x06;
const MpegSampleDescription::StreamType STREAM_TYPE_IPMP      = 0x07;
const MpegSampleDescription::StreamType STREAM_TYPE_OCI       = 0x08;
const MpegSampleDescription::StreamType STREAM_TYPE_MPEGJ     = 0x09;
const MpegSampleDescription::StreamType STREAM_TYPE_TEXT      = 0x0D;

const MpegSampleDescription::OTI OTI_MPEG4_SYSTEM         = 0x01;
const MpegSampleDescription::OTI OTI_MPEG4_SYSTEM_COR     = 0x02;
const MpegSampleDescription::OTI OTI_MPEG4_TEXT           = 0x08;
const MpegSampleDescription::OTI OTI_MPEG4_VISUAL         = 0x20;
const MpegSampleDescription::OTI OTI_MPEG4_AUDIO          = 0x40;
const MpegSampleDescription::OTI OTI_MPEG2_VISUAL_SIMPLE  = 0x60;
const MpegSampleDescription::OTI OTI_MPEG2_VISUAL_MAIN    = 0x61;
const MpegSampleDescription::OTI OTI_MPEG2_VISUAL_SNR     = 0x62;
const MpegSampleDescription::OTI OTI_MPEG2_VISUAL_SPATIAL = 0x63;
const MpegSampleDescription::OTI OTI_MPEG2_VISUAL_HIGH    = 0x64;
const MpegSampleDescription::OTI OTI_MPEG2_VISUAL_422     = 0x65;
const MpegSampleDescription::OTI OTI_MPEG2_AAC_AUDIO_MAIN = 0x66;
const MpegSampleDescription::OTI OTI_MPEG2_AAC_AUDIO_LC   = 0x67;
const MpegSampleDescription::OTI OTI_MPEG2_AAC_AUDIO_SSRP = 0x68;
const MpegSampleDescription::OTI OTI_MPEG2_PART3_AUDIO    = 0x69;
const MpegSampleDescription::OTI OTI_MPEG1_VISUAL         = 0x6A;
const MpegSampleDescription::OTI OTI_MPEG1_AUDIO          = 0x6B;
const MpegSampleDescription::OTI OTI_JPEG                 = 0x6C;
const MpegSampleDescription::OTI OTI_PNG                  = 0x6D;
const MpegSampleDescription::OTI OTI_JPEG2000             = 0x6E;
const MpegSampleDescription::OTI OTI_EVRC_VOICE           = 0xA0;
const MpegSampleDescription::OTI OTI_SMV_VOICE            = 0xA1;
const MpegSampleDescription::OTI OTI_3GPP2_CMF            = 0xA2;
const MpegSampleDescription::OTI OTI_SMPTE_VC1            = 0xA3;
const MpegSampleDescription::OTI OTI_DIRAC_VIDEO          = 0xA4;
const MpegSampleDescription::OTI OTI_AC3_AUDIO            = 0xA5;
const MpegSampleDescription::OTI OTI_EAC3_AUDIO           = 0xA6;
const MpegSampleDescription::OTI OTI_DRA_AUDIO            = 0xA7;
const MpegSampleDescription::OTI OTI_G719_AUDIO           = 0xA8;
const MpegSampleDescription::OTI OTI_DTS_AUDIO            = 0xA9;
const MpegSampleDescription::OTI OTI_DTS_HIRES_AUDIO      = 0xAA;
const MpegSampleDescription::OTI OTI_DTS_MASTER_AUDIO     = 0xAB;
const MpegSampleDescription::OTI OTI_DTS_EXPRESS_AUDIO    = 0xAC;
const MpegSampleDescription::OTI OTI_13K_VOICE            = 0xE1;

const UI08 MPEG4_AUDIO_OBJECT_TYPE_AAC_MAIN              = 1;  /**< AAC Main Profile                             */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_AAC_LC                = 2;  /**< AAC Low Complexity                           */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_AAC_SSR               = 3;  /**< AAC Scalable Sample Rate                     */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_AAC_LTP               = 4;  /**< AAC Long Term Predictor                      */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_SBR                   = 5;  /**< Spectral Band Replication                    */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_AAC_SCALABLE          = 6;  /**< AAC Scalable                                 */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_TWINVQ                = 7;  /**< Twin VQ                                      */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_CELP                  = 8;  /**< CELP                                         */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_HVXC                  = 9;  /**< HVXC                                         */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_TTSI                  = 12; /**< TTSI                                         */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_MAIN_SYNTHETIC        = 13; /**< Main Synthetic                               */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_WAVETABLE_SYNTHESIS   = 14; /**< WavetableSynthesis                           */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_GENERAL_MIDI          = 15; /**< General MIDI                                 */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_ALGORITHMIC_SYNTHESIS = 16; /**< Algorithmic Synthesis                        */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_LC             = 17; /**< Error Resilient AAC Low Complexity           */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_LTP            = 19; /**< Error Resilient AAC Long Term Prediction     */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_SCALABLE       = 20; /**< Error Resilient AAC Scalable                 */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_ER_TWINVQ             = 21; /**< Error Resilient Twin VQ                      */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_ER_BSAC               = 22; /**< Error Resilient Bit Sliced Arithmetic Coding */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_LD             = 23; /**< Error Resilient AAC Low Delay                */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_ER_CELP               = 24; /**< Error Resilient CELP                         */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_ER_HVXC               = 25; /**< Error Resilient HVXC                         */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_ER_HILN               = 26; /**< Error Resilient HILN                         */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_ER_PARAMETRIC         = 27; /**< Error Resilient Parametric                   */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_SSC                   = 28; /**< SSC                                          */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_PS                    = 29; /**< Parametric Stereo                            */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_MPEG_SURROUND         = 30; /**< MPEG Surround                                */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_LAYER_1               = 32; /**< MPEG Layer 1                                 */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_LAYER_2               = 33; /**< MPEG Layer 2                                 */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_LAYER_3               = 34; /**< MPEG Layer 3                                 */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_DST                   = 35; /**< DST Direct Stream Transfer                   */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_ALS                   = 36; /**< ALS Lossless Coding                          */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_SLS                   = 37; /**< SLS Scalable Lossless Coding                 */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_SLS_NON_CORE          = 38; /**< SLS Sclable Lossless Coding Non-Core         */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_ELD            = 39; /**< Error Resilient AAC ELD                      */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_SMR_SIMPLE            = 40; /**< SMR Simple                                   */
const UI08 MPEG4_AUDIO_OBJECT_TYPE_SMR_MAIN              = 41; /**< SMR Main                                     */

}
#endif //MP4_SAMPLEDESCRIPTION_H
