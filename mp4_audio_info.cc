//
// Created by wlanjie on 2018/3/16.
//

#include "mp4_audio_info.h"
#include "sampledescription.h"

namespace mp4 {
// TODO name
const unsigned int AAC_MAX_SAMPLING_FREQUENCY_INDEX = 12;
static const unsigned int AacSamplingFreqTable[13] =
{
        96000, 88200, 64000, 48000,
        44100, 32000, 24000, 22050,
        16000, 12000, 11025, 8000,
        7350
};

Mp4AudioDecoderConfig::Mp4AudioDecoderConfig() {
    reset();
}

Result Mp4AudioDecoderConfig::parse(const UI08 *data, Size dataSize) {
    Result result;
    Mp4AudioDsiParser bits(data, dataSize);

    // default config
    reset();

    // parse the audio object type
    result = parseAudioObjectType(bits, objectType);
    if (FAILED(result)) {
        return result;
    }

    // parse the sampling frequency
    result = parseSamplingFrequency(bits, samplingFrequencyIndex, samplingFrequency);
    if (FAILED(result)) {
        return result;
    }

    if (bits.bitsLeft() < 4) {
        return ERROR_INVALID_FORMAT;
    }
    channelConfiguration = (ChannelConfiguration) bits.readBits(4);
    channelCount = (unsigned int) channelConfiguration;
    if (channelCount == 7) {
        channelCount = 8;
    } else if (channelCount > 7) {
        channelCount = 0;
    }

    if (objectType == MPEG4_AUDIO_OBJECT_TYPE_SBR ||
        objectType == MPEG4_AUDIO_OBJECT_TYPE_PS) {
        Extension.objectType = MPEG4_AUDIO_OBJECT_TYPE_SBR;
        Extension.sbrPresent = true;
        Extension.psPresent  = objectType == MPEG4_AUDIO_OBJECT_TYPE_PS;
        result = parseSamplingFrequency(bits, Extension.samplingFrequencyIndex, Extension.samplingFrequency);
        if (FAILED(result)) {
            return result;
        }
        result = parseAudioObjectType(bits, objectType);
        if (FAILED(result)) {
            return result;
        }
        if (objectType == MPEG4_AUDIO_OBJECT_TYPE_ER_BSAC) {
            if (bits.bitsLeft() < 4) {
                return ERROR_INVALID_FORMAT;
            }
            bits.readBits(4); // extensionChannelConfiguration (4)
        }
    } else {
        Extension.objectType             = 0;
        Extension.samplingFrequency      = 0;
        Extension.samplingFrequencyIndex = 0;
        Extension.sbrPresent             = false;
        Extension.psPresent              = false;
    }

    switch (objectType) {
        case MPEG4_AUDIO_OBJECT_TYPE_AAC_MAIN:
        case MPEG4_AUDIO_OBJECT_TYPE_AAC_LC:
        case MPEG4_AUDIO_OBJECT_TYPE_AAC_SSR:
        case MPEG4_AUDIO_OBJECT_TYPE_AAC_LTP:
        case MPEG4_AUDIO_OBJECT_TYPE_AAC_SCALABLE:
        case MPEG4_AUDIO_OBJECT_TYPE_TWINVQ:
        case MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_LC:
        case MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_LTP:
        case MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_SCALABLE:
        case MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_LD:
        case MPEG4_AUDIO_OBJECT_TYPE_ER_TWINVQ:
        case MPEG4_AUDIO_OBJECT_TYPE_ER_BSAC:
            result = parseGASpecificInfo(bits);
            if (result == SUCCESS) {
                if (Extension.objectType !=  MPEG4_AUDIO_OBJECT_TYPE_SBR &&
                    bits.bitsLeft() >= 16) {
                    result = parseExtension(bits);
                }
            }
            if (result == ERROR_NOT_SUPPORTED) {
                // not a fatal error
                result = SUCCESS;
            }
            if (result != SUCCESS) {
                return result;
            }
            break;

        default:
            return ERROR_NOT_SUPPORTED;
    }

    return SUCCESS;
    return SUCCESS;
}

void Mp4AudioDecoderConfig::reset() {
    objectType = 0;
    samplingFrequencyIndex = 0;
    samplingFrequency = 0;
    channelCount = 0;
    channelConfiguration = CHANNEL_CONFIG_NONE;
    frameLengthFlag = false;
    dependsOnCoreCoder = false;
    coreCoderDelay = 0;
    Extension.sbrPresent = false;
    Extension.psPresent = false;
    Extension.objectType = 0;
    Extension.samplingFrequencyIndex = 0;
    Extension.samplingFrequency = 0;
}

Result Mp4AudioDecoderConfig::parseAudioObjectType(Mp4AudioDsiParser &parser, UI08 &objectType) {
    if (parser.bitsLeft() < 5) {
        return ERROR_INVALID_FORMAT;
    }
    objectType = (UI08) parser.readBits(5);
    if ((int)objectType == 31) {
        if (parser.bitsLeft() < 6) {
            return ERROR_INVALID_FORMAT;
        }
        objectType = (UI08)(32 + parser.readBits(6));
    }
    return SUCCESS;
}

Result Mp4AudioDecoderConfig::parseGASpecificInfo(Mp4AudioDsiParser &parser) {
    if (parser.bitsLeft() < 2) {
        return ERROR_INVALID_FORMAT;
    }
    frameLengthFlag = (parser.readBits(1) == 1);
    dependsOnCoreCoder = (parser.readBits(1) == 1);
    if (dependsOnCoreCoder) {
        if (parser.bitsLeft() < 14) {
            return ERROR_INVALID_FORMAT;
        }
        coreCoderDelay = parser.readBits(14);
    } else {
        coreCoderDelay = 0;
    }
    if (parser.bitsLeft() < 1) {
        return ERROR_INVALID_FORMAT;
    }
    unsigned int extensionFlag = parser.readBits(1);
    if (channelConfiguration == CHANNEL_CONFIG_NONE) {
        /*program_config_element (); */
        return ERROR_NOT_SUPPORTED;
    }
    if (objectType == MPEG4_AUDIO_OBJECT_TYPE_AAC_SCALABLE ||
        objectType == MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_SCALABLE) {
        if (parser.bitsLeft() < 3) {
            return ERROR_INVALID_FORMAT;
        }
        parser.readBits(3); // layerNr
    }
    if (extensionFlag) {
        if (objectType == MPEG4_AUDIO_OBJECT_TYPE_ER_BSAC) {
            if (parser.bitsLeft() < 16) {
                return ERROR_INVALID_FORMAT;
            }
            parser.readBits(16); // numOfSubFrame (5); layer_length (11)
        }
        if (objectType == MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_LC       ||
            objectType == MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_SCALABLE ||
            objectType == MPEG4_AUDIO_OBJECT_TYPE_ER_AAC_LD) {
            if (parser.bitsLeft() < 3) {
                return ERROR_INVALID_FORMAT;
            }
            parser.readBits(3); // aacSectionDataResilienceFlag (1)
            // aacScalefactorDataResilienceFlag (1)
            // aacSpectralDataResilienceFlag (1)
        }
        if (parser.bitsLeft() < 1) {
            return ERROR_INVALID_FORMAT;
        }
        unsigned int extensionFlag3 = parser.readBits(1);
        if (extensionFlag3) {
            return ERROR_NOT_SUPPORTED;
        }
    }

    return SUCCESS;
}

Result Mp4AudioDecoderConfig::parseSamplingFrequency(Mp4AudioDsiParser &parser, unsigned int &samplingFrequencyIndex,
                                                     unsigned int &samplingFrequency) {
    if (parser.bitsLeft() < 4) {
        return ERROR_INVALID_FORMAT;
    }

    samplingFrequencyIndex = parser.readBits(4);
    if (samplingFrequencyIndex == 0xF) {
        if (parser.bitsLeft() < 24) {
            return ERROR_INVALID_FORMAT;
        }
        samplingFrequency = parser.readBits(24);
    } else if (samplingFrequencyIndex <= AAC_MAX_SAMPLING_FREQUENCY_INDEX) {
        samplingFrequency = AacSamplingFreqTable[samplingFrequencyIndex];
    } else {
        samplingFrequency = 0;
        return ERROR_INVALID_FORMAT;
    }
    return SUCCESS;
}

Result Mp4AudioDecoderConfig::parseExtension(Mp4AudioDsiParser &parser) {
    if (parser.bitsLeft() < 16) {
        return ERROR_INVALID_FORMAT;
    }
    unsigned int sync_extension_type = parser.readBits(11);
    if (sync_extension_type == 0x2b7) {
        Result result = parseAudioObjectType(parser, Extension.objectType);
        if (FAILED(result)) return result;
        if (Extension.objectType == MPEG4_AUDIO_OBJECT_TYPE_SBR) {
            Extension.sbrPresent = (parser.readBits(1) == 1);
            if (Extension.sbrPresent) {
                result = parseSamplingFrequency(parser, Extension.samplingFrequencyIndex, Extension.samplingFrequency);
                if (FAILED(result)) return result;
                if (parser.bitsLeft() >= 12) {
                    sync_extension_type = parser.readBits(11);
                    if (sync_extension_type == 0x548) {
                        Extension.psPresent = (parser.readBits(1) == 1);
                    }
                }
            }
        } else if (Extension.objectType == MPEG4_AUDIO_OBJECT_TYPE_ER_BSAC) {
            Extension.sbrPresent = (parser.readBits(1) == 1);
            if (Extension.sbrPresent) {
                result = parseSamplingFrequency(parser, Extension.samplingFrequencyIndex, Extension.samplingFrequency);
                if (FAILED(result)) return result;
            }
            parser.readBits(4); // extensionChannelConfiguration
        }
    }
    return SUCCESS;
}

}
