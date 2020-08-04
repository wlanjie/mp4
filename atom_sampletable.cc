//
// Created by wlanjie on 2018/3/11.
//

#include "atom_sampletable.h"

namespace mp4 {

AtomSampleTable::AtomSampleTable(Container *stbl, ByteStream &stream) : stream(stream) {
    stsc = DYNAMIC_CAST(Stsc, stbl->getChild(ATOM_TYPE_STSC));
    stco = DYNAMIC_CAST(Stco, stbl->getChild(ATOM_TYPE_STCO));
    stsz = DYNAMIC_CAST(Stsz, stbl->getChild(ATOM_TYPE_STSZ));
    stz2 = DYNAMIC_CAST(Stz2, stbl->getChild(ATOM_TYPE_STZ2));
    ctts = DYNAMIC_CAST(Ctts, stbl->getChild(ATOM_TYPE_CTTS));
    stts = DYNAMIC_CAST(Stts, stbl->getChild(ATOM_TYPE_STTS));
    stss = DYNAMIC_CAST(Stss, stbl->getChild(ATOM_TYPE_STSS));
    stsd = DYNAMIC_CAST(Stsd, stbl->getChild(ATOM_TYPE_STSD));
    co64 = DYNAMIC_CAST(Co64, stbl->getChild(ATOM_TYPE_CO64));
    stream.addReference();
}

AtomSampleTable::~AtomSampleTable() {
    stream.release();
}

Result AtomSampleTable::getSample(Ordinal sampleIndex, Sample &sample) {
    if (!stsc) {
        return ERROR_INVALID_FORMAT;
    }
    if (!stco && !co64) {
        return ERROR_INVALID_FORMAT;
    }
    sampleIndex++;
    Ordinal chunk, skip, desc;
    Result result = stsc->getChunkForSample(sampleIndex, chunk, skip, desc);
    if (FAILED(result)) {
        return result;
    }
    if (skip > sampleIndex) {
        return ERROR_INTERNAL;
    }
    UI64 offset;
    if (stco) {
        UI32 offset32;
        result = stco->getChunkOffset(chunk, offset32);
        offset = offset32;
    } else {
        result = co64->getChunkOffset(chunk, offset);
    }
    if (FAILED(result)) {
        return result;
    }
    for (unsigned int i = sampleIndex - skip; i < sampleIndex; i++) {
        Size size = 0;
        if (stsz) {
            result = stsz->getSampleSize(i, size);
        } else if (stz2) {
            result = stz2->getSampleSize(i, size);
        } else {
            result = ERROR_INVALID_FORMAT;
        }
        if (FAILED(result)) {
            return result;
        }
        offset += size;
    }
    sample.setDescriptionIndex(desc - 1);
    UI32 ctsOffset = 0;
    UI64 dts = 0;
    UI32 duration = 0;
    if (stts) {
        result = stts->getDts(sampleIndex, dts, &duration);
        if (FAILED(result)) {
            return result;
        }
    }
    sample.setDuration(duration);
    sample.setDts(dts);
    if (!ctts) {
        sample.setCts(dts);
    } else {
        result = ctts->getCtsOffset(sampleIndex, ctsOffset);
        if (FAILED(result)) {
            return result;
        }
        sample.setCtsDelta(ctsOffset);
    }
    Size sampleSize = 0;
    if (stsz) {
        result = stsz->getSampleSize(sampleIndex, sampleSize);
    } else if (stz2) {
        result = stz2->getSampleSize(sampleIndex, sampleSize);
    } else {
        result = ERROR_INVALID_FORMAT;
    }
    if (FAILED(result)) {
        return result;
    }
    sample.setSize(sampleSize);
    if (!stss) {
        sample.setSync(true);
    } else {
        sample.setSync(stss->isSampleSync(sampleIndex));
    }
    sample.setOffset(offset);
    sample.setDataStream(stream);
    return SUCCESS;
}

Cardinal AtomSampleTable::getSampleCount() {
    if (stsz) {
        return stsz->getSampleCount();
    } else if (stz2) {
        return stz2->getSampleCount();
    }
    return 0;
}

SampleDescription *AtomSampleTable::getSampleDescription(Ordinal index) {
    return stsd ? stsd->getSampleDescription(index) : nullptr;
}

Cardinal AtomSampleTable::getSampleDescriptionCount() {
    return stsd ? stsd->getSampleDescriptionCount() : 0;
}

Result AtomSampleTable::getSampleChunkPosition(Ordinal sampleIndex, Ordinal &chunkIndex, Ordinal &positionInChunk) {
    chunkIndex = 0;
    positionInChunk = 0;
    Ordinal sampleDescriptionIndex;
    return getChunkForSample(sampleIndex, chunkIndex, positionInChunk, sampleDescriptionIndex);
}

Result AtomSampleTable::getSampleIndexForTimeStamp(UI64 ts, Ordinal &sampleIndex) {
    return stts ? stts->getSampleIndexForTimeStamp(ts, sampleIndex) : FAILURE;
}

Result AtomSampleTable::getSampleTimeStamp(Ordinal sampleIndex, UI64& ts) {
    UI32 ctsOffset = 0;
    UI32 duration = 0;
    Result result = SUCCESS;
    ts = 0;
    sampleIndex++;
    if (stts) {
        result = stts->getDts(sampleIndex, ts, &duration);
        if (FAILED(result)) {
            return result;
        }
    }
    if (ctts) {
        result = ctts->getCtsOffset(sampleIndex, ctsOffset);
        if (FAILED(result)) {
            return result;
        }
        ts += ctsOffset;
    }
    return result;
}

Ordinal AtomSampleTable::getNearestSyncSampleIndex(Ordinal index, bool before) {
    if (!stss) {
        return index;
    }
    index += 1;
    Cardinal entryCount = stss->getEntries().ItemCount();
    if (before) {
        Ordinal cursor = 0;
        for (unsigned int i = 0; i < entryCount; i++) {
            if (stss->getEntries()[i] >= index) {
                return cursor;
            }
            if (stss->getEntries()[i]) {
                cursor = stss->getEntries()[i] - 1;
            }
        }
        return cursor;
    } else {
        for (unsigned int i = 0; i < entryCount; i++) {
            if (stss->getEntries()[i] >= index) {
                return stss->getEntries()[i] ? stss->getEntries()[i] - 1 : index - 1;
            }
        }
    }
    return getSampleCount();
}

Result AtomSampleTable::getChunkForSample(Ordinal sampleIndex, Ordinal &chunkIndex, Ordinal &positionInChunk,
                                          Ordinal &sampleDescriptionIndex) {
    chunkIndex = 0;
    positionInChunk = 0;
    sampleDescriptionIndex = 0;
    if (!stsc) {
        return ERROR_INVALID_STATE;
    }
    Ordinal chunk = 0;
    Result result = stsc->getChunkForSample(sampleIndex + 1, chunk, positionInChunk, sampleDescriptionIndex);
    if (FAILED(result)) {
        return result;
    }
    if (chunk == 0) {
        return ERROR_INTERNAL;
    }
    chunkIndex = chunk - 1;
    return SUCCESS;
}

Result AtomSampleTable::getChunkOffset(Ordinal chunkIndex, Position &offset) {
    if (stco) {
        UI32 offset32;
        Result result = stco->getChunkOffset(chunkIndex + 1, offset32);
        if (SUCCEEDED(result)) {
            offset = offset32;
        } else {
            offset = 0;
        }
        return result;
    } else if (co64) {
        return co64->getChunkOffset(chunkIndex + 1, offset);
    }
    offset = 0;
    return FAILURE;
}

Result AtomSampleTable::setChunkOffset(Ordinal chunkIndex, Position offset) {
    if (stco) {
        if ((offset >> 32) != 0) {
            return ERROR_OUT_OF_RANGE;
        }
        return stco->setChunkOffset(chunkIndex + 1, (UI32) offset);
    } else if (co64) {
        return co64->setChunkOffset(chunkIndex + 1, offset);
    }
    return FAILURE;
}

Result AtomSampleTable::setSampleSize(Ordinal sampleIndex, Size size) {
    if (stsz) {
        return stsz->setSampleSize(sampleIndex + 1, size);
    } else if (stz2) {
        return stz2->setSampleSize(sampleIndex + 1, size);
    }
    return FAILURE;
}

}
