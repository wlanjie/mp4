//
// Created by wlanjie on 2018/3/11.
//

#include "co64.h"
#include "utils.h"

Co64 *Co64::create(Size size, ByteStream &stream) {
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
    return new Co64(size, version, flags, stream);
}

Co64::Co64(UI32 size, UI08 version, UI32 flags, ByteStream &stream) : Atom(ATOM_TYPE_CO64, size, version, flags) {
    stream.readUI32(entryCount);
    if (entryCount > (size - FULL_ATOM_HEADER_SIZE - 4) / 8) {
        entryCount = (size - FULL_ATOM_HEADER_SIZE - 4) / 8;
    }
    entries = new UI64(entryCount);
    for (Ordinal i = 0; i < entryCount; i++) {
        stream.readUI64(entries[i]);
    }
}

Co64::Co64(UI64 *offsets, UI32 offsetCount) :
        Atom(ATOM_TYPE_CO64, FULL_ATOM_HEADER_SIZE + 4 + offsetCount * 8, 0, 0),
        entries(new UI64[offsetCount]),
        entryCount(offsetCount) {
    CopyMemory(entries, offsets, entryCount * 8);
}

Co64::~Co64() {
    delete[] entries;
}

Result Co64::writeFields(ByteStream &stream) {
    Result result = stream.writeUI32(entryCount);
    if (FAILED(result)) {
        return result;
    }
    for (Ordinal i = 0; i < entryCount; i++) {
        result = stream.writeUI64(entries[i]);
        if (FAILED(result)) {
            return result;
        }
    }
    return SUCCESS;
}

Result Co64::getChunkOffset(Ordinal chunk, UI64 &chunkOffset) {
    if (chunk > entryCount || chunk == 0) {
        return ERROR_OUT_OF_RANGE;
    }
    chunkOffset = entries[chunk - 1];
    return SUCCESS;
}

Result Co64::setChunkOffset(Ordinal chunk, UI64 chunkOffset) {
    if (chunk > entryCount || chunk == 0) {
        return ERROR_OUT_OF_RANGE;
    }
    entries[chunk - 1] = chunkOffset;
    return SUCCESS;
}

Result Co64::adjustChunkOffset(SI64 delta) {
    for (Ordinal i = 0; i < entryCount; i++) {
        entries[i] += delta;
    }
    return SUCCESS;
}
