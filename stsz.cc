//
// Created by wlanjie on 2018/3/11.
//

#include "stsz.h"
#include "utils.h"

Stsz *Stsz::create(Size size, ByteStream &stream) {
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
    return new Stsz(size, version, flags, stream);
}

Stsz::Stsz(UI32 size, UI08 version, UI32 flags, ByteStream &stream) : Atom(ATOM_TYPE_STSZ, size, version, flags) {
    stream.readUI32(sampleSize);
    stream.readUI32(sampleCount);
    if (sampleSize == 0) {
        if (sampleCount > (size - 8) / 4) {
            sampleCount = 0;
            return;
        }
        Cardinal sampleCount = this->sampleCount;
        entries.SetItemCount(sampleCount);
        unsigned char* buffer = new unsigned char[sampleCount * 4];
        Result result = stream.read(buffer, sampleCount * 4);
        if (FAILED(result)) {
            delete[] buffer;
            return;
        }
        for (unsigned int i = 0; i < sampleCount; i++) {
            entries[i] = BytesToUInt32BE(&buffer[i * 4]);
        }
        delete[] buffer;
    }
}

Stsz::Stsz() :
        Atom(ATOM_TYPE_STSZ, FULL_ATOM_HEADER_SIZE + 8, 0, 0),
        sampleCount(0),
        sampleSize(0) {

}

Result Stsz::writeFields(ByteStream &stream) {
    Result result = stream.writeUI32(sampleSize);
    if (FAILED(result)) {
        return result;
    }
    result = stream.writeUI32(sampleCount);
    if (FAILED(result)) {
        return result;
    }
    if (sampleSize == 0) {
        for (UI32 i = 0; i < sampleCount; i++) {
            result = stream.writeUI32(entries[i]);
            if (FAILED(result)) {
                return result;
            }
        }
    }
    return SUCCESS;
}

UI32 Stsz::getSampleCount() {
    return sampleCount;
}

Result Stsz::getSampleSize(Ordinal sample, Size &sampleSize) {
    if (sample > sampleCount || sample == 0) {
        sampleSize = 0;
        return ERROR_OUT_OF_RANGE;
    } else {
        if (this->sampleSize != 0) {
            sampleSize = this->sampleSize;
        } else {
            sampleSize = entries[sample - 1];
        }
    }
    return SUCCESS;
}

Result Stsz::setSampleSize(Ordinal sample, Size sampleSize) {
    if (sample > sampleCount || sample == 0) {
        return ERROR_OUT_OF_RANGE;
    } else {
        if (entries.ItemCount() == 0) {
            if (sampleSize != this->sampleSize) {
                if (sample == 1) {
                    this->sampleSize = sampleSize;
                    return SUCCESS;
                } else {
                    return ERROR_INVALID_PARAMETERS;
                }
            }
        } else {
            entries[sample - 1] = sampleSize;
        }
        return SUCCESS;
    }
}

Result Stsz::addEntry(UI32 size) {
    entries.Append(size);
    sampleCount++;
    size32 += 4;
    return SUCCESS;
}
