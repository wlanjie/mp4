//
// Created by wlanjie on 2018/3/13.
//

#include "stz2.h"
#include "utils.h"

Stz2 *Stz2::create(Size size, ByteStream &stream) {
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
    return new Stz2(size, version, flags, stream);
}

Stz2::Stz2(UI32 size, UI08 version, UI32 flags, ByteStream &stream) :
        Atom(ATOM_TYPE_STZ2, size, version, flags) {
    UI08 reserved;
    stream.readUI08(reserved);
    stream.readUI08(reserved);
    stream.readUI08(reserved);
    stream.readUI08(fieldsSize);
    stream.readUI32(sampleCount);
    if (fieldsSize != 4 && fieldsSize != 8 && fieldsSize != 16) {
        return;
    }
    Cardinal sampleCount = this->sampleCount;
    entries.SetItemCount(sampleCount);
    unsigned int tableSize = (sampleCount * fieldsSize + 7) / 8;
    if ((tableSize + 8) > size) {
        return;
    }
    unsigned char* buffer = new unsigned char[tableSize];
    Result result = stream.read(buffer, tableSize);
    if (FAILED(result)) {
        delete[] buffer;
        return;
    }
    switch (fieldsSize) {
        case 4:
            for (unsigned int i = 0; i < sampleCount; i++) {
                if ((i % 2) == 0) {
                    entries[i] = (buffer[i / 2] >> 4) & 0x0F;
                } else {
                    entries[i] = buffer[i / 2] & 0x0F;
                }
            }
            break;
        case 8:
            for (unsigned int i = 0; i < sampleCount; i++) {
                entries[i] = buffer[i];
            }
            break;
        case 16:
            for (unsigned int i = 0; i < sampleCount; i++) {
                entries[i] = BytesToUInt16BE(&buffer[i * 2]);
            }
            break;
    }
    delete[] buffer;
}

Stz2::Stz2(UI08 fieldSize) :
        Atom(ATOM_TYPE_STZ2, FULL_ATOM_HEADER_SIZE + 8, 0, 0),
        fieldsSize(fieldSize),
        sampleCount(0) {
    if (fieldSize != 4 && fieldSize != 8 && fieldSize != 16) {
        this->fieldsSize = 16;
    }
}

Result Stz2::writeFields(ByteStream &stream) {
    Result result = stream.writeUI08(0);
    if (FAILED(result)) {
        return result;
    }
    result = stream.writeUI08(0);
    if (FAILED(result)) {
        return result;
    }
    result = stream.writeUI08(0);
    if (FAILED(result)) {
        return result;
    }
    result = stream.writeUI08(fieldsSize);
    if (FAILED(result)) {
        return result;
    }
    result = stream.writeUI32(sampleCount);
    if (FAILED(result)) {
        return result;
    }
    switch (fieldsSize) {
        case 4:
            for (UI32 i = 0; i < sampleCount; i++) {
                if (i + 1 < sampleCount) {
                    result = stream.writeUI08(((entries[i] & 0x0F) << 4) | (entries[i + 1] & 0x0F));
                } else {
                    result = stream.writeUI08((entries[i] & 0x0F) << 4);
                }
                if (FAILED(result)) {
                    return result;
                }
            }
            break;
        case 8:
            for (UI32 i = 0; i < sampleCount; i++) {
                result = stream.writeUI08((UI08) entries[i]);
                if (FAILED(result)) {
                    return result;
                }
            }
            break;
        case 16:
            for (UI32 i = 0; i < sampleCount; i++) {
                result = stream.writeUI16((UI16) entries[i]);
                if (FAILED(result)) {
                    return result;
                }
            }
            break;
    }
    return 0;
}

UI32 Stz2::getSampleCount() {
    return sampleCount;
}

Result Stz2::getSampleSize(Ordinal sample, Size &sampleSize) {
    if (sample > sampleCount || sample == 0) {
        sampleSize = 0;
        return ERROR_OUT_OF_RANGE;
    } else {
        sampleSize = entries[sample - 1];
        return SUCCESS;
    }
}

Result Stz2::setSampleSize(Ordinal sample, Size sampleSize) {
    if (sample > sampleCount || sample == 0) {
        return ERROR_OUT_OF_RANGE;
    } else {
        entries[sample - 1] = sampleSize;
        return SUCCESS;
    }
}

Result Stz2::addEntry(UI32 size) {
    entries.Append(size);
    sampleCount++;
    if (fieldsSize == 4) {
        if ((sampleCount % 2) == 1) {
            size32++;
        }
    } else {
        size32 += fieldsSize / 8;
    }
    return 0;
}
