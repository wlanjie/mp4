//
// Created by wlanjie on 2018/3/11.
//

#include "avcc.h"
#include "utils.h"

namespace mp4 {

Avcc *Avcc::create(Size size, ByteStream &stream) {
    unsigned int payloadSize = size - ATOM_HEADER_SIZE;
    DataBuffer payloadData(payloadSize);
    Result result = stream.read(payloadData.useData(), payloadSize);
    if (FAILED(result)) {
        return nullptr;
    }
    const UI08* payload = payloadData.useData();
    if (payload[0] != 1) {
        return nullptr;
    }
    if (payloadSize < 6) {
        return nullptr;
    }
    unsigned int numSeqParams = payload[5] & 31;
    unsigned int cursor = 6;
    for (unsigned int i = 0; i < numSeqParams; i++) {
        if (cursor + 2 > payloadSize) {
            return nullptr;
        }
        cursor += 2 + BytesToInt16BE(&payload[cursor]);
        if (cursor > payloadSize) {
            return nullptr;
        }
    }
    unsigned int numPicParams = payload[cursor++];
    if (cursor > payloadSize) {
        return nullptr;
    }
    for (unsigned int i = 0; i < numPicParams; i++) {
        if (cursor + 2 > payloadSize) {
            return nullptr;
        }
        cursor += 2 + BytesToInt16BE(&payload[cursor]);
        if (cursor > payloadSize) {
            return nullptr;
        }
    }
    return new Avcc(size, payload);
}

Avcc::Avcc(UI32 size, const UI08 *payload) : Atom(ATOM_TYPE_AVCC, size) {
    unsigned int payloadSize = size - ATOM_HEADER_SIZE;
    rawBytes.setData(payload, payloadSize);
    configurationVersion = payload[0];
    profile = payload[1];
    profileCompatibility = payload[2];
    level = payload[3];
    naluLengthSize = 1 + (payload[4] & 3);
    UI08 numSeqParams = payload[5] & 31;
    sequenceParameters.EnsureCapacity(numSeqParams);
    unsigned int cursor = 6;
    for (unsigned int i = 0; i < numSeqParams; i++) {
        if (cursor + 2 <= payloadSize) {
            UI16 paramLength = BytesToInt16BE(&payload[cursor]);
            cursor += 2;
            if (cursor + paramLength <= payloadSize) {
                sequenceParameters.Append(DataBuffer());
                sequenceParameters[i].setData(&payload[cursor], paramLength);
                cursor += paramLength;
            }
        }
    }
    UI08 numPicParams = payload[cursor++];
    pictureParamters.EnsureCapacity(numPicParams);
    for (unsigned int i = 0; i < numPicParams; i++) {
        if (cursor + 2 <= payloadSize) {
            UI16 paramLength = BytesToInt16BE(&payload[cursor]);
            cursor += 2;
            if (cursor + paramLength <= payloadSize) {
                pictureParamters.Append(DataBuffer());
                pictureParamters[i].setData(&payload[cursor], paramLength);
                cursor += paramLength;
            }
        }
    }
}

Avcc::Avcc() :
        Atom(ATOM_TYPE_AVCC, ATOM_HEADER_SIZE),
        configurationVersion(1),
        profile(0),
        level(0),
        profileCompatibility(0),
        naluLengthSize(0) {
    updateRawBytes();
    size32 += rawBytes.getDataSize();
}

Avcc::Avcc(UI08 profile,
           UI08 level,
           UI08 profileCompatibility,
           UI08 lengthSize,
           Array<DataBuffer> &sequenceParameters,
           Array<DataBuffer> &pictureParameters) :
        Atom(ATOM_TYPE_AVCC, ATOM_HEADER_SIZE),
        configurationVersion(1),
        profile(profile),
        level(level),
        profileCompatibility(profileCompatibility),
        naluLengthSize(lengthSize) {
    for (unsigned int i = 0; i < sequenceParameters.ItemCount(); i++) {
        this->sequenceParameters.Append(sequenceParameters[i]);
    }
    for (unsigned int i = 0; i < pictureParameters.ItemCount(); i++) {
        this->pictureParamters.Append(pictureParameters[i]);
    }

    updateRawBytes();
    size32 += rawBytes.getDataSize();
}

Avcc::Avcc(const Avcc &other) :
        Atom(ATOM_TYPE_AVCC, other.size32),
        configurationVersion(other.configurationVersion),
        profile(other.profile),
        level(other.level),
        profileCompatibility(other.profileCompatibility),
        naluLengthSize(other.naluLengthSize),
        rawBytes(other.rawBytes) {
    for (unsigned int i = 0; i < other.sequenceParameters.ItemCount(); i++) {
        sequenceParameters.Append(other.sequenceParameters[i]);
    }
    for (unsigned int i = 0; i < other.pictureParamters.ItemCount(); i++) {
        pictureParamters.Append(other.pictureParamters[i]);
    }
}

const char *Avcc::getProfileName(UI08 profile) {
    switch (profile) {
        case AVC_PROFILE_BASELINE: return "Baseline";
        case AVC_PROFILE_MAIN:     return "Main";
        case AVC_PROFILE_EXTENDED: return "Extended";
        case AVC_PROFILE_HIGH:     return "High";
        case AVC_PROFILE_HIGH_10:  return "High 10";
        case AVC_PROFILE_HIGH_422: return "High 4:2:2";
        case AVC_PROFILE_HIGH_444: return "High 4:4:4";
    }

    return nullptr;
}

Result Avcc::writeFields(ByteStream &stream) {
    return stream.write(rawBytes.getData(), rawBytes.getDataSize());
}

void Avcc::updateRawBytes() {
    unsigned int payloadSize = 6;
    for (unsigned int i = 0; i < sequenceParameters.ItemCount(); i++) {
        payloadSize += 2 + sequenceParameters[i].getDataSize();
    }
    ++payloadSize;
    for (unsigned int i = 0; i < pictureParamters.ItemCount(); i++) {
        payloadSize += 2 + pictureParamters[i].getDataSize();
    }
    rawBytes.setDataSize(payloadSize);
    UI08* payload = rawBytes.useData();
    payload[0] = configurationVersion;
    payload[1] = profile;
    payload[2] = profileCompatibility;
    payload[3] = level;
    payload[4] = 0xFC | (naluLengthSize - 1);
    payload[5] = 0xE0 | (UI08) sequenceParameters.ItemCount();

    unsigned int cursor = 6;
    for (unsigned int i = 0; i < sequenceParameters.ItemCount(); i++) {
        UI16 paramLength = (UI16) sequenceParameters[i].getDataSize();
        BytesFromUInt16BE(&payload[cursor], paramLength);
        cursor += 2;
        CopyMemory(&payload[cursor], sequenceParameters[i].getData(), paramLength);
        cursor += paramLength;
    }
    payload[cursor++] = (UI08) pictureParamters.ItemCount();
    for (unsigned int i = 0; i < pictureParamters.ItemCount(); i++) {
        UI16 paramLength = (UI16) pictureParamters[i].getDataSize();
        BytesFromUInt16BE(&payload[cursor], paramLength);
        cursor += 2;
        CopyMemory(&payload[cursor], pictureParamters[i].getData(), paramLength);
        cursor += paramLength;
    }
}

}
