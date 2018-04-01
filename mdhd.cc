//
// Created by wlanjie on 2018/3/9.
//

#include "mdhd.h"
#include "utils.h"

namespace mp4 {

Mdhd *Mdhd::create(Size size, ByteStream &stream) {
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
    return new Mdhd(size, version, flags, stream);
}

Mdhd::Mdhd(UI32 creationTime,
                   UI32 modificationTime,
                   UI32 timeScale,
                   UI64 duration,
                   const char *language) :
        Atom(ATOM_TYPE_MDHD, FULL_ATOM_HEADER_SIZE + 20, 0, 0),
        creationTime(creationTime),
        modificationTime(modificationTime),
        timeScale(timeScale),
        duration(duration) {
    this->language.assign(language, 3);
    if (duration > 0xFFFFFFFF) {
        version = 1;
        size32 += 12;
    }
}

Mdhd::Mdhd(UI32 size, UI08 version, UI32 flags, ByteStream &stream) :
        Atom(ATOM_TYPE_MDHD, size, version, flags) {
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
    };
    unsigned char lang[2];
    stream.read(lang, 2);
    char l0 = ((lang[0]>>2)&0x1F);
    char l1 = (((lang[0]&0x3)<<3) | ((lang[1]>>5)&0x7));
    char l2 = ((lang[1]&0x1F));
    if (l0 && l1 && l2) {
        char lang_str[3] = {(char)(l0+0x60), (char)(l1+0x60), (char)(l2+0x60)};
        language.assign(lang_str, 3);
    } else {
        language.assign("```", 3);
    }
}

Result Mdhd::writeFields(ByteStream &stream) {
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
    return 0;
}

UI32 Mdhd::getDurationMs() const {
    return durationMsFromUnits(duration, timeScale);
}

}
