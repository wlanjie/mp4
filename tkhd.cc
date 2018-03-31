//
// Created by wlanjie on 2018/3/9.
//

#include "tkhd.h"

namespace mp4 {

Tkhd *Tkhd::create(Size size, ByteStream &stream) {
    UI08 version;
    UI32 flags;
    if (size < FULL_ATOM_HEADER_SIZE) {
        return nullptr;
    }
    if (FAILED(Atom::readFullHeader(stream, version, flags))) {
        return nullptr;
    }
    if (version > 1) {
        return nullptr;
    }
    return new Tkhd(size, version, flags, stream);
}

Tkhd::Tkhd(UI32 creationTime,
                   UI32 modificationTime,
                   UI32 trackId,
                   UI64 duration,
                   UI16 volume,
                   UI32 width,
                   UI32 height,
                   UI16 layer,
                   UI16 alternateGroup,
                   const SI32 *matrix) :
        Atom(ATOM_TYPE_TKHD, FULL_ATOM_HEADER_SIZE + 80, 0, 0),
        creationTime(creationTime),
        modificationTime(modificationTime),
        trackId(trackId),
        duration(duration),
        volume(volume),
        width(width),
        height(height),
        layer(layer),
        alternateGroup(alternateGroup),
        reserved1(0),
        reserved3(0) {
    flags = TKHD_FLAG_DEFAULTS;
    if (matrix) {
        for (int i = 0; i < 9; ++i) {
            this->matrix[i] = matrix[i];
        }
    } else {
        this->matrix[0] = 0x00010000;
        this->matrix[1] = 0;
        this->matrix[2] = 0;
        this->matrix[3] = 0;
        this->matrix[4] = 0x00010000;
        this->matrix[5] = 0;
        this->matrix[6] = 0;
        this->matrix[7] = 0;
        this->matrix[8] = 0x40000000;
    }
    reserved2[0] = 0;
    reserved2[1] = 0;
    if (duration > 0xFFFFFFFF) {
        version = 1;
        size32 += 12;
    }
}

Tkhd::Tkhd(UI32 size, UI08 version, UI32 flags, ByteStream &stream)
        : Atom(ATOM_TYPE_TKHD, size, version, flags) {
    if (this->version == 0) {
        UI32 creationTime;
        stream.readUI32(creationTime);
        this->creationTime = creationTime;
        UI32 modificationTime;
        stream.readUI32(modificationTime);
        this->modificationTime = modificationTime;
        stream.readUI32(trackId);
        stream.readUI32(reserved1);
        UI32 duration;
        stream.readUI32(duration);
        this->duration = duration;
    } else {
        stream.readUI64(creationTime);
        stream.readUI64(modificationTime);
        stream.readUI32(trackId);
        stream.readUI32(reserved1);
        stream.readUI64(duration);
    }
    stream.read((void *) reserved2, 8);
    stream.readUI16(layer);
    stream.readUI16(alternateGroup);
    stream.readUI16(volume);
    stream.readUI16(reserved3);
    for (int i = 0; i < 9; ++i) {
        UI32 coefficient = 0;
        stream.readUI32(coefficient);
        matrix[i] = (SI32) coefficient;
    }
    stream.readUI32(width);
    stream.readUI32(height);
}

Result Tkhd::writeFields(ByteStream &stream) {
    Result result;
    if (version == 0) {
        result = stream.writeUI32((UI32)creationTime);
        if (FAILED(result)) return result;
        result = stream.writeUI32((UI32) modificationTime);
        if (FAILED(result)) return result;
        result = stream.writeUI32(trackId);
        if (FAILED(result)) return result;
        result = stream.writeUI32(reserved1);
        if (FAILED(result)) return result;
        result = stream.writeUI32((UI32) duration);
        if (FAILED(result)) return result;
    } else {
        result = stream.writeUI64(creationTime);
        if (FAILED(result)) return result;
        result = stream.writeUI64(modificationTime);
        if (FAILED(result)) return result;
        result = stream.writeUI32(trackId);
        if (FAILED(result)) return result;
        result = stream.writeUI32(reserved1);
        if (FAILED(result)) return result;
        result = stream.writeUI64(duration);
        if (FAILED(result)) return result;
    }
    result = stream.write(reserved2, sizeof(reserved2));
    if (FAILED(result)) return result;
    result = stream.writeUI16(layer);
    if (FAILED(result)) return result;
    result = stream.writeUI16(alternateGroup);
    if (FAILED(result)) return result;
    result = stream.writeUI16(volume);
    if (FAILED(result)) return result;
    result = stream.writeUI16(reserved3);
    if (FAILED(result)) return result;
    for (int i = 0; i < 9; ++i) {
        result = stream.writeUI32((UI32)matrix[i]);
        if (FAILED(result)) return result;
    }
    result = stream.writeUI32(width);
    if (FAILED(result)) return result;
    result = stream.writeUI32(height);
    if (FAILED(result)) return result;
    return 0;
}

}
