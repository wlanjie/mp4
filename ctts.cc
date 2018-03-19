//
// Created by wlanjie on 2018/3/11.
//

#include "ctts.h"
#include "utils.h"

Ctts *Ctts::create(Size size, ByteStream &stream) {
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
    return new Ctts(size, version, flags, stream);
}

Ctts::Ctts(UI32 size, UI08 version, UI32 flags, ByteStream &stream) :
        Atom(ATOM_TYPE_CTTS, size, version, flags) {
    LookupCache.sample = 0;
    LookupCache.entryIndex = 0;
    UI32 entryCount;
    stream.readUI32(entryCount);
    entries.SetItemCount(entryCount);
    unsigned char* buffer = new unsigned char[entryCount * 8];
    Result result = stream.read(buffer, entryCount * 8);
    if (FAILED(result)) {
        delete[] buffer;
        return;
    }
    for (unsigned i = 0; i < entryCount; i++) {
        entries[i].sampleCount = BytesToUInt32BE(&buffer[i * 8]);
        UI32 offset = BytesToUInt32BE(&buffer[i * 8 + 4]);
        entries[i].sampleOffset = offset;
    }
    delete[] buffer;
}

Ctts::Ctts() : Atom(ATOM_TYPE_CTTS, FULL_ATOM_HEADER_SIZE + 4, 0, 0) {
    LookupCache.sample = 0;
    LookupCache.entryIndex = 0;
}

Result Ctts::writeFields(ByteStream &stream) {
    Cardinal entryCount = entries.ItemCount();
    Result result = stream.writeUI32(entryCount);
    if (FAILED(result)) {
        return result;
    }
    for (Ordinal i = 0; i < entryCount; i++) {
        result = stream.writeUI32(entries[i].sampleCount);
        if (FAILED(result)) {
            return result;
        }
        result = stream.writeUI32(entries[i].sampleOffset);
        if (FAILED(result)) {
            return result;
        }
    }
    return SUCCESS;
}

Result Ctts::addEntry(UI32 count, UI32 ctsOffset) {
    entries.Append(CttsTableEntry(count, ctsOffset));
    size32 += 8;
    return SUCCESS;
}

Result Ctts::getCtsOffset(Ordinal sample, UI32 &ctsOffset) {
    ctsOffset = 0;
    if (sample == 0) {
        return ERROR_OUT_OF_RANGE;
    }
    Ordinal lookupStart = 0;
    Ordinal sampleStart = 0;
    if (sample >= LookupCache.sample) {
        lookupStart = LookupCache.entryIndex;
        sampleStart = LookupCache.sample;
    }
    for (Ordinal i = lookupStart; i < entries.ItemCount(); i++) {
        CttsTableEntry& entry = entries[i];
        if (sample <= sampleStart + entry.sampleCount) {
            ctsOffset = entry.sampleOffset;
            LookupCache.entryIndex = i;
            LookupCache.sample = sampleStart;
            return SUCCESS;
        }
        sampleStart += entry.sampleCount;
    }
    return ERROR_OUT_OF_RANGE;
}
