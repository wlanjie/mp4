//
// Created by wlanjie on 2018/3/10.
//

#include "stts.h"
#include "utils.h"

Stts *Stts::create(Size size, ByteStream &stream) {
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
    return new Stts(size, version, flags, stream);
}

Stts::Stts() : Atom(ATOM_TYPE_STTS, FULL_ATOM_HEADER_SIZE + 4, 0, 0) {
    LookupCache.entryIndex = 0;
    LookupCache.sample = 0;
    LookupCache.dts = 0;
}

Stts::Stts(UI32 size, UI08 version, UI32 flags, ByteStream &stream) :
        Atom(ATOM_TYPE_STTS, size, version, flags) {
    LookupCache.entryIndex = 0;
    LookupCache.sample = 0;
    LookupCache.dts = 0;
    UI32 entryCount;
    stream.readUI32(entryCount);
    while (entryCount--) {
        UI32 sampleCount;
        UI32 sampleDuration;
        if (stream.readUI32(sampleCount) == SUCCESS && stream.readUI32(sampleDuration) == SUCCESS) {
            entries.Append(SttsTableEntry(sampleCount, sampleDuration));
        }
    }
}

Result Stts::getDts(Ordinal sample, UI64 &dts, UI32 *duration) {
    dts = 0;
    if (duration) {
        *duration = 0;
    }
    if (sample == 0) {
        return ERROR_OUT_OF_RANGE;
    }
    --sample;
    Ordinal lookupStart = 0;
    Ordinal sampleStart = 0;
    UI64 dtsStart = 0;
    if (sample >= LookupCache.sample) {
        lookupStart = LookupCache.entryIndex;
        sampleStart = LookupCache.sample;
        dtsStart = LookupCache.dts;
    }
    for (Ordinal i = lookupStart; i < entries.ItemCount(); i++) {
        SttsTableEntry& entry = entries[i];
        if (sample < sampleStart + entry.sampleCount) {
            dts = dtsStart + (UI64) (sample - sampleStart) * (UI64) entry.sampleDuration;
            if (duration) {
                *duration = entry.sampleDuration;
            }
            LookupCache.entryIndex = i;
            LookupCache.sample = sampleStart;
            LookupCache.dts = dtsStart;
            return SUCCESS;
        }
        sampleStart += entry.sampleCount;
        dtsStart += entry.sampleCount * entry.sampleDuration;
    }
    return ERROR_OUT_OF_RANGE;
}

Result Stts::writeFields(ByteStream &stream) {
    Result result;
    Cardinal entryCount = entries.ItemCount();
    result = stream.writeUI32(entryCount);
    if (FAILED(result)) {
        return result;
    }
    for (Ordinal i = 0; i < entryCount; i++) {
        result = stream.writeUI32(entries[i].sampleCount);
        if (FAILED(result)) {
            return result;
        }
        result = stream.writeUI32(entries[i].sampleDuration);
        if (FAILED(result)) {
            return result;
        }
    }
    return SUCCESS;
}

Result Stts::addEntry(UI32 sampleCount, UI32 sampleDuration) {
    entries.Append(SttsTableEntry(sampleCount, sampleDuration));
    size32 += 8;
    return SUCCESS;
}

Result Stts::getSampleIndexForTimeStamp(UI64 ts, Ordinal &sampleIndex) {
    Cardinal entryCount = entries.ItemCount();
    UI64 accumulated = 0;
    sampleIndex = 0;
    for (Ordinal i = 0; i < entryCount; i++) {
        UI64 nextAccumulated = accumulated + (UI64) entries[i].sampleCount * (UI64) entries[i].sampleDuration;
        if (ts < nextAccumulated) {
            sampleIndex += (UI32) ((ts - accumulated) / entries[i].sampleDuration);
            return SUCCESS;
        }
        accumulated = nextAccumulated;
        sampleIndex += entries[i].sampleCount;
    }
    return FAILURE;
}