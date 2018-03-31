//
// Created by wlanjie on 2018/3/8.
//

#include "mvhd.h"
#include "utils.h"

namespace mp4 {

Mvhd *Mvhd::create(Size size, ByteStream &stream) {
    UI08 version;
    UI32 flags;
    if (size < FULL_ATOM_HEADER_SIZE) {
        return nullptr;
    }
    if FAILED(Atom::readFullHeader(stream, version, flags)) {
        return nullptr;
    }
    if (version > 1) {
        return nullptr;
    }
    return new Mvhd(size, version, flags, stream);
}

Mvhd::Mvhd(UI32 creationTime, UI32 modificationTime, UI32 timeScale, UI64 duration, UI32 rate, UI16 volume) :
        Atom(ATOM_TYPE_MVHD, FULL_ATOM_HEADER_SIZE + 96, 0, 0),
        creationTime(creationTime),
        modificationTime(modificationTime),
        timeScale(timeScale),
        duration(duration),
        rate(rate),
        volume(volume),
        nextTrackId(0xFFFFFFFF) {
    matrix[0] = 0x00010000;
    matrix[1] = 0;
    matrix[2] = 0;
    matrix[3] = 0;
    matrix[4] = 0x00010000;
    matrix[5] = 0;
    matrix[6] = 0;
    matrix[7] = 0;
    matrix[8] = 0x40000000;

    SetMemory(reserved1, 0, sizeof(reserved1));
    SetMemory(reserved2, 0, sizeof(reserved2));
    SetMemory(predefined, 0, sizeof(predefined));
    if (duration > 0xFFFFFFFF) {
        version = 1;
        size32 += 12;
    }
}

Mvhd::Mvhd(UI32 size, UI08 version, UI32 flags, ByteStream &stream) :
        Atom(ATOM_TYPE_MVHD, size, version, flags) {
    if (this->version == 0) {
        UI32 creationTime;
        stream.readUI32(creationTime);
        this->creationTime = creationTime;
        UI32 modificationTime;
        stream.readUI32(modificationTime);
        this->modificationTime = modificationTime;
        stream.readUI32(timeScale);
        UI32 duration;
        stream.readUI32(duration);
        this->duration = duration;
    } else {
        stream.readUI64(creationTime);
        stream.readUI64(modificationTime);
        stream.readUI32(timeScale);
        stream.readUI64(duration);
    }
    stream.readUI32(rate);
    stream.readUI16(volume);
    stream.read(reserved1, sizeof(reserved1));
    stream.read(reserved2, sizeof(reserved2));

    for (int i = 0; i < 9; ++i) {
        stream.readUI32(matrix[i]);
    }
    stream.read(predefined, sizeof(predefined));
    stream.readUI32(nextTrackId);
}

Result Mvhd::writeFields(ByteStream &stream) {
    Result result;
    if (version == 0) {
        result = stream.writeUI32((UI32) creationTime);
        if (FAILED(result)) return result;
        result = stream.writeUI32((UI32) modificationTime);
        if (FAILED(result)) return result;
        result = stream.writeUI32(timeScale);
        if (FAILED(result)) return result;
        result = stream.writeUI32((UI32) duration);
        if (FAILED(result)) return result;
    } else {
        result = stream.writeUI64(creationTime);
        if (FAILED(result)) return result;
        result = stream.writeUI64(modificationTime);
        if (FAILED(result)) return result;
        result = stream.writeUI32(timeScale);
        if (FAILED(result)) return result;
        result = stream.writeUI64(duration);
        if (FAILED(result)) return result;
    }
    result = stream.writeUI32(rate);
    if (FAILED(result)) return result;
    result = stream.writeUI16(volume);
    if (FAILED(result)) return result;
    result = stream.write(reserved1, sizeof(reserved1));
    if (FAILED(result)) return result;
    result = stream.write(reserved2, sizeof(reserved2));
    if (FAILED(result)) return result;

    for (int i = 0; i < 9; ++i) {
        result = stream.writeUI32(matrix[i]);
        if (FAILED(result)) return result;
    }
    result = stream.write(predefined, sizeof(predefined));
    if (FAILED(result)) return result;
    return stream.writeUI32(nextTrackId);
}

UI32 Mvhd::getDurationMs() {
    return timeScale ? (UI32) ConvertTime(duration, timeScale, 1000) : 0;
}

}
