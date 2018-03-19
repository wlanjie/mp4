//
// Created by wlanjie on 2018/3/10.
//

#include "sample.h"

Sample::Sample() :
        dataStream(nullptr),
        offset(0),
        size(0),
        duration(0),
        descriptionIndex(0),
        dts(0),
        ctsDelta(0),
        sync(true) {

}

Sample::Sample(const Sample &other) :
        dataStream(other.dataStream),
        offset(other.offset),
        size(other.size),
        duration(other.duration),
        descriptionIndex(other.descriptionIndex),
        dts(other.dts),
        ctsDelta(other.ctsDelta),
        sync(other.sync) {
    ADD_REFERENCE(dataStream);
}

Sample::Sample(ByteStream &stream,
               Position offset,
               Size size,
               UI32 duration,
               Ordinal descriptionIndex,
               UI64 dts,
               UI32 ctsDelta,
               bool syncFlag) :
        offset(offset),
        size(size),
        duration(duration),
        descriptionIndex(descriptionIndex),
        dts(dts),
        ctsDelta(ctsDelta),
        sync(syncFlag) {
    dataStream = &stream;
    ADD_REFERENCE(dataStream);
}

Sample::~Sample() {
    RELEASE(dataStream);
}

Sample &Sample::operator=(const Sample &other) {
    ADD_REFERENCE(other.dataStream);
    RELEASE(dataStream);
    dataStream = other.dataStream;
    offset = other.offset;
    size = other.size;
    duration = other.duration;
    descriptionIndex = other.descriptionIndex;
    dts = other.dts;
    ctsDelta = other.ctsDelta;
    sync = other.sync;
    return *this;
}

Result Sample::readData(DataBuffer &data) {
    return readData(data, size);
}

Result Sample::readData(DataBuffer &data, Size size, Size offset) {
    if (dataStream == nullptr) {
        return FAILURE;
    }
    if (size == 0) {
        return SUCCESS;
    }
    if (this->size < size + offset) {
        return FAILURE;
    }
    Result result = data.setDataSize(size);
    if (FAILED(result)) {
        return result;
    }
    result = dataStream->seek(this->offset + offset);
    if (FAILED(result)) {
        return result;
    }
    return dataStream->read(data.useData(), size);
}

void Sample::detach() {
    RELEASE(dataStream);
    dataStream = nullptr;
}

ByteStream *Sample::getDataStream() {
    ADD_REFERENCE(dataStream);
    return dataStream;
}

void Sample::setDataStream(ByteStream &stream) {
    RELEASE(dataStream);
    dataStream = &stream;
    ADD_REFERENCE(dataStream);
}

void Sample::reset() {
    RELEASE(dataStream);
    offset = 0;
    size = 0;
    duration = 0;
    descriptionIndex = 0;
    dts = 0;
    ctsDelta = 0;
    sync = false;
}
