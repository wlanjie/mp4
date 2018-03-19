//
// Created by wlanjie on 2018/3/10.
//

#include "sampletable.h"
#include "stsd.h"
#include "stts.h"
#include "stsc.h"
#include "stsz.h"
#include "stss.h"
#include "ctts.h"
#include "stco.h"
#include "co64.h"

Result SampleTable::generateStbl(Container *&stbl) {
    stbl = new Container(ATOM_TYPE_STBL);
    auto* stsd = new Stsd(this);
    auto* stts = new Stts();
    auto* stsc = new Stsc();
    auto* stsz = new Stsz();
    auto* stss = new Stss();
    Ctts* ctts = nullptr;

    Ordinal currentChunkIndex = 0;
    Size currentChunkSize = 0;
    Position currentChunkOffset = 0;
    Cardinal currentSamplesInChunk = 0;
    Ordinal currentSampleDescriptionIndex = 0;
    UI32 currentDuration = 0;
    Cardinal currentDurationRun = 0;
    UI32 currentCtsDelta = 0;
    Cardinal currentCtsDeltaRun = 0;
    Array<Position> chunkOffsets;
    bool allSampleAreSync = false;
    Cardinal sampleCount = getSampleCount();
    for (Ordinal i = 0; i < sampleCount; i++) {
        Sample sample;
        getSample(i, sample);
        UI32 newDuration = sample.getDuration();
        if (newDuration != currentDuration && currentDurationRun != 0) {
            stts->addEntry(currentDurationRun, currentDuration);
            currentDurationRun = 0;
        }
        ++currentDurationRun;
        currentDuration = newDuration;
        UI32 newCtsDelta = sample.getCtsDelta();
        if (newCtsDelta != currentCtsDelta && currentCtsDeltaRun != 0) {
            if (ctts == nullptr) {
                ctts = new Ctts();
            }
            ctts->addEntry(currentCtsDeltaRun, currentCtsDelta);
            currentCtsDeltaRun = 0;
        }
        ++currentCtsDeltaRun;
        currentCtsDelta = newCtsDelta;
        stsz->addEntry(sample.getSize());

        if (sample.isSync()) {
            stss->addEntry(i + 1);
            if (i == 0) {
                allSampleAreSync = true;
            }
        } else {
            allSampleAreSync = false;
        }

        Ordinal chunkIndex = 0;
        Ordinal positionInChunk = 0;
        Result result = getSampleChunkPosition(i, chunkIndex, positionInChunk);
        if (SUCCEEDED(result)) {
            if (chunkIndex != currentChunkIndex && currentSamplesInChunk != 0) {
                chunkOffsets.Append(currentChunkOffset);
                currentChunkOffset += currentChunkSize;
                stsc->addEntry(1, currentSamplesInChunk, currentSampleDescriptionIndex + 1);
                currentSamplesInChunk = 0;
                currentChunkSize = 0;
            }
            currentChunkIndex = chunkIndex;
        }
        currentSampleDescriptionIndex = sample.getDescriptionIndex();
        currentChunkSize += sample.getSize();
        ++currentSamplesInChunk;
    }
    if (sampleCount) {
        stts->addEntry(currentDurationRun, currentDuration);
    }
    if (ctts) {
        ASSERT(currentCtsDeltaRun != 0);
        ctts->addEntry(currentCtsDeltaRun, currentCtsDelta);
    }
    if (currentSamplesInChunk != 0) {
        chunkOffsets.Append(currentChunkOffset);
        stsc->addEntry(1, currentSamplesInChunk, currentSampleDescriptionIndex + 1);
    }
    stbl->addChild(stsd);
    stbl->addChild(stts);
    if (ctts) {
        stbl->addChild(ctts);
    }
    stbl->addChild(stsc);
    stbl->addChild(stsz);
    if (!allSampleAreSync && stss->getEntries().ItemCount() != 0) {
        stbl->addChild(stss);
    } else {
        delete stss;
    }
    Size chunkCount = chunkOffsets.ItemCount();
    if (currentChunkOffset <= 0xFFFFFFFF) {
        UI32* chunkOffset32 = new UI32[chunkCount];
        for (unsigned int i = 0; i < chunkCount; i++) {
            chunkOffset32[i] = (UI32) chunkOffsets[i];
        }
        Stco* stco = new Stco(&chunkOffset32[0], chunkCount);
        stbl->addChild(stco);
        delete[] chunkOffset32;
    } else {
        Co64* co64 = new Co64(&chunkOffsets[0], chunkCount);
        stbl->addChild(co64);
    }
    return SUCCESS;
}