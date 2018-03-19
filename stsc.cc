//
// Created by wlanjie on 2018/3/10.
//

#include "stsc.h"
#include "utils.h"

Stsc *Stsc::create(Size size, ByteStream &stream) {
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
    return new Stsc(size, version, flags, stream);
}

Stsc::Stsc() : Atom(ATOM_TYPE_STSC, FULL_ATOM_HEADER_SIZE + 4, 0, 0), cachedChunkGroup(0) {

}

Stsc::Stsc(UI32 size, UI08 version, UI32 flags, ByteStream &stream) :
        Atom(ATOM_TYPE_STSC, size, version, flags),
        cachedChunkGroup(0) {
    UI32 firstSample = 1;
    UI32 entryCount;
    if (size - ATOM_HEADER_SIZE < 4) {
        return;
    }
    stream.readUI32(entryCount);
    if ((size - ATOM_HEADER_SIZE - 4) / 12 < entryCount) {
        return;
    }
    entries.SetItemCount(entryCount);
    unsigned char* buffer = new unsigned char[entryCount * 12];
    Result result = stream.read(buffer, entryCount * 12);
    if (FAILED(result)) {
        delete[] buffer;
        return;
    }
    for (unsigned int i = 0; i < entryCount; i++) {
        UI32 firstChunk = BytesToUInt32BE(&buffer[i * 12]);
        UI32 samplesPerChunk = BytesToUInt32BE(&buffer[i * 12 + 4]);
        UI32 sampleDescriptionIndex = BytesToUInt32BE(&buffer[i * 12 + 8]);
        if (i) {
            Ordinal prev = i - 1;
            entries[prev].chunkCount = firstChunk - entries[prev].firstChunk;
            firstSample += entries[prev].chunkCount * entries[prev].samplesPerChunk;
        }
        entries[i].chunkCount = 0;
        entries[i].firstChunk = firstChunk;
        entries[i].firstSample = firstSample;
        entries[i].samplesPerChunk = samplesPerChunk;
        entries[i].sampleDescriptionIndex = sampleDescriptionIndex;
    }
    delete[] buffer;
}

Result Stsc::writeFields(ByteStream &stream) {
    Cardinal entryCount = entries.ItemCount();
    Result result = stream.writeUI32(entryCount);
    for (Ordinal i = 0; i < entryCount; i++) {
        result = stream.writeUI32(entries[i].firstChunk);
        if (FAILED(result)) {
            return result;
        }
        result = stream.writeUI32(entries[i].samplesPerChunk);
        if (FAILED(result)) {
            return result;
        }
        result = stream.writeUI32(entries[i].sampleDescriptionIndex);
        if (FAILED(result)) {
            return result;
        }
    }
    return 0;
}

Result Stsc::getChunkForSample(Ordinal sample, Ordinal &chunk, Ordinal &skip, Ordinal &sampleDescriptionIndex) {
    ASSERT(sample > 0);
    Ordinal group;
    if (cachedChunkGroup < entries.ItemCount() && entries[cachedChunkGroup].firstSample <= sample) {
        group = cachedChunkGroup;
    } else {
        group = 0;
    }
    while (group < entries.ItemCount()) {
        Cardinal sampleCount = entries[group].chunkCount * entries[group].samplesPerChunk;
        if (sampleCount == 0) {
            if (entries[group].firstSample > sample) {
                return ERROR_INVALID_FORMAT;
            }
        } else {
            if (entries[group].firstSample + sampleCount <= sample) {
                group++;
                continue;
            }
        }

        if (entries[group].samplesPerChunk == 0) {
            return ERROR_INVALID_FORMAT;
        }
        unsigned int chunkOffset = ((sample - entries[group].firstSample) / entries[group].samplesPerChunk);
        chunk = entries[group].firstChunk + chunkOffset;
        skip = sample - (entries[group].firstSample + entries[group].samplesPerChunk * chunkOffset);
        sampleDescriptionIndex = entries[group].sampleDescriptionIndex;

        cachedChunkGroup = group;
        return SUCCESS;
    }
    return 0;
}

Result Stsc::addEntry(Cardinal chunkCount, Cardinal samplePerChunk, Ordinal sampleDescriptionIndex) {
    Ordinal firstChunk;
    Ordinal firstSample;
    Cardinal entryCount = entries.ItemCount();
    if (entryCount == 0) {
        firstChunk = 1;
        firstSample = 1;
    } else {
        firstChunk = entries[entryCount - 1].firstChunk + entries[entryCount - 1].chunkCount;
        firstSample = entries[entryCount - 1].firstChunk +
                entries[entryCount - 1].chunkCount *
                        entries[entryCount - 1].samplesPerChunk;
    }
    entries.Append(StscTableEntry(firstChunk, firstSample, chunkCount, samplePerChunk, sampleDescriptionIndex));
    size32 += 12;
    return SUCCESS;
}
