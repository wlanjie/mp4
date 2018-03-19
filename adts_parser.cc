//
// Created by wlanjie on 2018/3/15.
//

#include "adts_parser.h"

#define ADTS_HEADER_SIZE 7

#define ADTS_SYNC_MASK     0xFFF6 /* 12 sync bits plus 2 layer bits */
#define ADTS_SYNC_PATTERN  0xFFF0 /* 12 sync bits=1 layer=0         */

const UI32
AdtsSamplingFrequencyTable[16] =
{
    96000,
    88200,
    64000,
    48000,
    44100,
    32000,
    24000,
    22050,
    16000,
    12000,
    11025,
    8000,
    7350,
    0,      /* Reserved */
    0,      /* Reserved */
    0       /* Escape code */
};

AdtsHeader::AdtsHeader(const UI08 *bytes) {
    id = ( bytes[1] & 0x08) >> 3;
    protectionAbsent =   bytes[1] & 0x01;
    profileObjectType = ( bytes[2] & 0xC0) >> 6;
    samplingFrequencyIndex = ( bytes[2] & 0x3C) >> 2;
    channelConfiguration = ((bytes[2] & 0x01) << 2) | ((bytes[3] & 0xC0) >> 6);
    frameLength = ((unsigned int)(bytes[3] & 0x03) << 11) | ((unsigned int)(bytes[4]) << 3) | ((unsigned int)(bytes[5] & 0xE0) >>  5);
    rawDataBlocks = bytes[6] & 0x03;
}

Result AdtsHeader::check() {
    if (samplingFrequencyIndex >= 0xD) {
        return FAILURE;
    }
    if (id == 1 && profileObjectType == 3) {
        return FAILURE;
    }
    return SUCCESS;
}

bool AdtsHeader::matchFixed(unsigned char *a, unsigned char *b) {
    if (a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && (a[3] & 0xF0) == (b[3] & 0xF0)) {
        return true;
    }
    return false;
}

AdtsParser::AdtsParser() : frameCount(0) {

}

AdtsParser::~AdtsParser() {

}

Result AdtsParser::reset() {
    frameCount = 0;
    return SUCCESS;
}

Result AdtsParser::feed(const UI08 *buffer, Size *bufferSize, Flags flags) {
    Size freeSpace;
    bits.m_Flags = flags;
    if (buffer == nullptr || bufferSize == nullptr || *bufferSize == 0) {
        return SUCCESS;
    }
    freeSpace = bits.GetBytesFree();
    if (*bufferSize > freeSpace) {
        *bufferSize = freeSpace;
    }
    if (*bufferSize == 0) {
        return SUCCESS;
    }
    return bits.WriteBytes(buffer, *bufferSize);
}

Result AdtsParser::findFrame(AacFrame &frame) {
    unsigned int available;
    unsigned char raw_header[ADTS_HEADER_SIZE];
    Result result;

    /* align to the start of the next byte */
    bits.ByteAlign();

    /* find a frame header */
    result = findHeader(raw_header);
    if (FAILED(result)) return result;

    /* parse the header */
    AdtsHeader adts_header(raw_header);

    /* check the header */
    result = adts_header.check();
    if (FAILED(result)) {
        return ERROR_CORRUPTED_BITSTREAM;
    }

    /* check if we have enough data to peek at the next header */
    available = bits.GetBytesAvailable();
    if (available >= adts_header.frameLength + ADTS_HEADER_SIZE) {
        // enough to peek at the header of the next frame
        unsigned char peek_raw_header[ADTS_HEADER_SIZE];

        bits.SkipBytes(adts_header.frameLength);
        bits.PeekBytes(peek_raw_header, ADTS_HEADER_SIZE);
        bits.SkipBytes(-((int)adts_header.frameLength));

        /* check the header */
        AdtsHeader peek_adts_header(peek_raw_header);
        result = peek_adts_header.check();
        if (FAILED(result)) {
            return ERROR_CORRUPTED_BITSTREAM;
        }

        /* check that the fixed part of this header is the same as the */
        /* fixed part of the previous header                           */
        if (!AdtsHeader::matchFixed(peek_raw_header, raw_header)) {
            return ERROR_CORRUPTED_BITSTREAM;
        }
    } else if (available < adts_header.frameLength || (bits.m_Flags & BITSTREAM_FLAG_EOS) == 0) {
        // not enough for a frame, or not at the end (in which case we'll want to peek at the next header)
        return ERROR_NOT_ENOUGH_DATA;
    }

    bits.SkipBytes(ADTS_HEADER_SIZE);

    /* fill in the frame info */
    frame.info.standard = (adts_header.id == 1 ? AAC_STANDARD_MPEG2 : AAC_STANDARD_MPEG4);
    switch (adts_header.profileObjectType) {
        case 0:
            frame.info.profile = AAC_PROFILE_MAIN;
            break;

        case 1:
            frame.info.profile = AAC_PROFILE_LC;
            break;

        case 2:
            frame.info.profile = AAC_PROFILE_SSR;
            break;

        case 3:
            frame.info.profile = AAC_PROFILE_LTP;
    }
    frame.info.frameLength = adts_header.frameLength - ADTS_HEADER_SIZE;
    frame.info.channelConfiguration = adts_header.channelConfiguration;
    frame.info.samplingFrequencyIndex = adts_header.samplingFrequencyIndex;
    frame.info.samplingFrequency = AdtsSamplingFrequencyTable[adts_header.samplingFrequencyIndex];

    /* skip crc if present */
    if (adts_header.protectionAbsent == 0) {
        bits.SkipBits(16);
    }

    /* set the frame source */
    frame.source = &bits;
    return SUCCESS;
}

Result AdtsParser::skip(Size size) {
    return 0;
}

Size AdtsParser::getBytesFree() {
    return bits.GetBytesFree();
}

Size AdtsParser::getBytesAvailable() {
    return bits.GetBytesAvailable();
}

Result AdtsParser::findHeader(UI08 *header) {
    Size available = bits.GetBytesAvailable();
    while (available-- >= ADTS_HEADER_SIZE) {
        bits.PeekBytes(header, 2);

        if ((((header[0] << 8) | header[1]) & ADTS_SYNC_MASK) == ADTS_SYNC_PATTERN) {
            /* found a sync pattern, read the entire the header */
            bits.PeekBytes(header, ADTS_HEADER_SIZE);

            return SUCCESS;
        } else {
            bits.ReadByte(); // skip
        }
    }
    return ERROR_NOT_ENOUGH_DATA;
}
