//
// Created by wlanjie on 2018/3/11.
//

#include "stss.h"
#include "utils.h"

namespace mp4 {

Stss *Stss::create(Size size, ByteStream &stream) {
    UI08 version;
    UI32 flags;
    if (size < FULL_ATOM_HEADER_SIZE) {
        return nullptr;
    }
    if (FAILED(Atom::readFullHeader(stream, version, flags))) {
        return nullptr;
    }
    if (version != 0) {
        return nullptr;
    }
    return new Stss(size, version, flags, stream);
}

Stss::Stss(UI32 size, UI08 version, UI32 flags, ByteStream &stream) :
        Atom(ATOM_TYPE_STSS, size, version, flags),
        LooupCache(0) {
    if (size - ATOM_HEADER_SIZE < 4) {
        return;
    }
    UI32 entryCount;
    stream.readUI32(entryCount);
    if ((size - ATOM_HEADER_SIZE - 4) / 4 < entryCount) {
        return;
    }
    unsigned char* buffer = new unsigned char[entryCount * 4];
    Result result = stream.read(buffer, entryCount * 4);
    if (FAILED(result)) {
        delete[] buffer;
        return;
    }
    entries.SetItemCount(entryCount);
    for (unsigned int i = 0; i < entryCount; i++) {
        entries[i] = BytesToUInt32BE(&buffer[i * 4]);
    }
    delete[] buffer;
}

Stss::Stss() : Atom(ATOM_TYPE_STSS, FULL_ATOM_HEADER_SIZE + 4, 0, 0) {

}

Result Stss::addEntry(UI32 sample) {
    entries.Append(sample);
    size32 += 4;
    return SUCCESS;
}

Result Stss::writeFields(ByteStream &stream) {
    Cardinal entryCount = entries.ItemCount();
    Result result = stream.writeUI32(entryCount);
    if (FAILED(result)) {
        return result;
    }
    for (Ordinal i = 0; i < entryCount; i++) {
        result = stream.writeUI32(entries[i]);
        if (FAILED(result)) {
            return result;
        }
    }
    return SUCCESS;
}

bool Stss::isSampleSync(Ordinal sample) {
    unsigned int entryIndex = 0;
    if (sample == 0 || entries.ItemCount() == 0) {
        return false;
    }
    if (entries[LooupCache] <= sample) {
        entryIndex = LooupCache;
    }
    while (entryIndex <= entries.ItemCount() && entries[entryIndex] <= sample) {
        if (entries[entryIndex] == sample) {
            LooupCache = entryIndex;
            return true;
        }
        entryIndex++;
    }
    return false;
}

}
