//
// Created by wlanjie on 2018/3/11.
//

#include "stco.h"
#include "utils.h"

Stco *Stco::create(Size size, ByteStream &stream) {
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
    return new Stco(size, version, flags, stream);
}

Stco::Stco(UI32 size, UI08 version, UI32 flags, ByteStream &stream) : Atom(ATOM_TYPE_STCO, size, version, flags) {
    stream.readUI32(entryCount);
    if (entryCount > (size - FULL_ATOM_HEADER_SIZE - 4) / 4) {
        entryCount = (size - FULL_ATOM_HEADER_SIZE - 4) / 4;
    }
    entries = new UI32[entryCount];
    unsigned char* buffer = new unsigned char[entryCount * 4];
    Result result = stream.read(buffer, entryCount * 4);
    if (FAILED(result)) {
        delete[] buffer;
        return;
    }
    for (Ordinal i = 0; i < entryCount; i++) {
        entries[i] = BytesToUInt32BE(&buffer[i * 4]);
    }
    delete[] buffer;
}

Stco::Stco(UI32 *offsets, UI32 offsetCount) :
        Atom(ATOM_TYPE_STCO, FULL_ATOM_HEADER_SIZE + 4 + offsetCount * 4, 0, 0),
        entries(new UI32[offsetCount]),
        entryCount(offsetCount) {
    CopyMemory(entries, offsets, entryCount * 4);
}

Stco::~Stco() {
    delete[] entries;
}

Result Stco::writeFields(ByteStream &stream) {
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

Result Stco::getChunkOffset(Ordinal chunk, UI32 &chunkOffsets) {
    if (chunk > entryCount || chunk == 0) {
        return ERROR_OUT_OF_RANGE;
    }
    chunkOffsets = entries[chunk - 1];
    return SUCCESS;
}

Result Stco::setChunkOffset(Ordinal chunk, UI32 chunkOffsets) {
    if (chunk > entryCount || chunk == 0) {
        return ERROR_OUT_OF_RANGE;
    }
    entries[chunk - 1] = chunkOffsets;
    return SUCCESS;
}

Result Stco::adjustChunkOffsets(int delta) {
    for (Ordinal i = 0; i < entryCount; i++) {
        entries[i] += delta;
    }
    return SUCCESS;
}
