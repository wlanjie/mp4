//
// Created by wlanjie on 2018/3/9.
//

#include "track.h"
#include "hdlr.h"
#include "atom_sampletable.h"
#include "synthetic_sampletable.h"
#include "utils.h"

namespace mp4 {

Track::Track(Track::Type type,
             SampleTable* sampleTable,
             UI32 trackId,
             UI32 movieTimeScale,
             UI64 trackDuration,
             UI32 mediaTimeScale,
             UI64 mediaDuration,
             const char *language,
             UI32 width,
             UI32 height) :
        trakAtomIsOwned(true),
        type(type),
        sampleTable(sampleTable),
        sampleTableIsOwned(true),
        movieTimeScale(movieTimeScale ? movieTimeScale : TRACK_DEFAULT_MOVIE_TIMESCALE) {

    unsigned int volume = 0;
    if (type == TYPE_AUDIO) {
        volume = 0x100;
    }
    Atom::Type hdlrType;
    const char* hdlrName;
    switch(type) {

        case TYPE_AUDIO:
            hdlrType = HANDLER_TYPE_SOUN;
            hdlrName = "Sound Handler";
            break;

        case TYPE_VIDEO:
            hdlrType = HANDLER_TYPE_VIDE;
            hdlrName = "Video Handler";
            break;

        default:
            hdlrType = 0;
            hdlrName = nullptr;
            break;
    }

    trakAtom = new Trak(
            sampleTable,
            hdlrType,
            hdlrName,
            trackId,
            0,
            0,
            trackDuration,
            mediaTimeScale,
            mediaDuration,
            (UI16) volume,
            language,
            width,
            height);
}

Track::Track(SampleTable* sampleTable,
             UI32 trackId,
             UI32 movieTimeScale,
             UI64 trackDuration,
             UI32 mediaTimeScale,
             UI64 mediaDuration,
             const Track* trackProtoType) :
        trakAtomIsOwned(true),
        type(trackProtoType->type),
        sampleTable(sampleTable),
        sampleTableIsOwned(true),
        movieTimeScale(movieTimeScale ? movieTimeScale : TRACK_DEFAULT_MOVIE_TIMESCALE) {
    Atom::Type hdlrType;
    const char* hdlrName;
    switch (trackProtoType->getType()) {
        case TYPE_VIDEO:
            hdlrType = HANDLER_TYPE_VIDE;
            hdlrName = "Video Handler";
            break;

        case TYPE_AUDIO:
            hdlrType = HANDLER_TYPE_SOUN;
            hdlrName = "Audio Handler";
            break;

        default:
            hdlrType = 0;
            hdlrName = nullptr;
            break;
    }
    const Tkhd* tkhd = nullptr;
    if (trackProtoType->getTrakAtom()) {
        tkhd = trackProtoType->getTrakAtom()->getTkhd();
    }
    trakAtom = new Trak(sampleTable,
                        hdlrType,
                        hdlrName,
                        trackId,
                        0,
                        0,
                        trackDuration,
                        mediaTimeScale,
                        mediaDuration,
                        tkhd ? tkhd->getVolume() : (trackProtoType->getType() == TYPE_AUDIO ? 0x100 : 0),
                        trackProtoType->getTrackLanguage(),
                        trackProtoType->getWidth(),
                        trackProtoType->getHeight(),
                        tkhd ? tkhd->getType() : 0,
                        tkhd ? tkhd->getAlternateGroup() : 0,
                        tkhd ? tkhd->getMatrix() : nullptr);
}

Track::Track(Trak &atom, ByteStream &stream, UI32 movieTimeScale) :
        trakAtom(&atom),
        trakAtomIsOwned(false),
        type(TYPE_UNKNOWN),
        sampleTable(nullptr),
        sampleTableIsOwned(true),
        movieTimeScale(movieTimeScale) {
    auto* sub = atom.findChild("mdia/hdlr");
    if (sub) {
        auto* hdlr = DYNAMIC_CAST(Hdlr, sub);
        if (hdlr) {
            UI32 type = hdlr->getHandlerType();
            if (type == HANDLER_TYPE_VIDE) {
                this->type = TYPE_VIDEO;
            } else if (type == HANDLER_TYPE_SOUN) {
                this->type = TYPE_AUDIO;
            }
        }
    }
    auto* stbl = DYNAMIC_CAST(Container, atom.findChild("mdia/minf/stbl"));
    if (stbl) {
        sampleTable = new AtomSampleTable(stbl, stream);
    }
}

Track::~Track() {
    if (trakAtomIsOwned) delete trakAtom;
    if (sampleTableIsOwned) delete sampleTable;
}

Track *Track::clone(Result *result) {
    auto* sampleTable = new SyntheticSampleTable();
    if (result) {
        *result = SUCCESS;
    }
    for (unsigned int i = 0; ; i++) {
        auto sampleDescription = getSampleDescription(i);
        if (!sampleDescription) {
            break;
        }
        sampleTable->addSampleDescription(sampleDescription->clone());
    }
    Sample sample;
    Ordinal index = 0;
    while (SUCCEEDED(getSample(index, sample))) {
        ByteStream* stream = sample.getDataStream();
        sampleTable->addSample(*stream,
                               sample.getOffset(),
                               sample.getSize(),
                               sample.getDuration(),
                               sample.getDescriptionIndex(),
                               sample.getDts(),
                               sample.getCtsDelta(),
                               sample.isSync());
        RELEASE(stream);
        index++;
    }
    auto* clone = new Track(sampleTable, getId(), getMovieTimeScale(), getDuration(), getMediaTimeScale(), getMediaDuration(), this);
    return clone;
}

UI32 Track::getFlags() const {
    if (trakAtom) {
        Tkhd* tkhdAtom = DYNAMIC_CAST(Tkhd, trakAtom->findChild("tkhd"));
        return tkhdAtom ? tkhdAtom->getFlags() : 0;
    }
    return 0;
}

void Track::setFlags(UI32 flags) {
    if (trakAtom) {
        Tkhd* tkhdAtom = DYNAMIC_CAST(Tkhd, trakAtom->findChild("tkhd"));
        if (tkhdAtom) {
            tkhdAtom->setFlags(flags);
        }
    }
}

UI32 Track::getHandlerType() const {
    if (trakAtom) {
        auto* hdlr = DYNAMIC_CAST(Hdlr, trakAtom->findChild("mdia/hdlr"));
        if (hdlr) {
            return hdlr->getHandlerType();
        }
    }
    return 0;
}

UI64 Track::getDuration() const {
    return trakAtom->getDuration();
}

UI32 Track::getDurationMs() const {
    auto duration = trakAtom->getDuration();
    return durationMsFromUnits(duration, movieTimeScale);
}

UI32 Track::getWidth() const {
    return trakAtom->getWidth();
}

UI32 Track::getHeight() const {
    return trakAtom->getHeight();
}

Cardinal Track::getSampleCount() const {
    return sampleTable ? sampleTable->getSampleCount() : 0;
}

Result Track::getSample(Ordinal index, Sample& sample) {
    return sampleTable ? sampleTable->getSample(index, sample) : FAILURE;
}

Result Track::readSample(Ordinal index, Sample& sample, DataBuffer& data) {
    auto result = getSample(index, sample);
    if (FAILED(result)) {
        return result;
    }
    return sample.readData(data);
}

Result Track:: getSampleIndexForTimeStampMs(UI64 tsMs, Ordinal& index) {
    auto ts = convertTime(tsMs, 1000, getMediaTimeScale());
    return sampleTable->getSampleIndexForTimeStamp(ts, index);
}

Ordinal Track::getNearestSyncSampleIndex(Ordinal index, bool before) {
    if (!sampleTable) {
        return index;
    }
    return sampleTable->getNearestSyncSampleIndex(index, before);
}

SampleDescription* Track::getSampleDescription(Ordinal index) {
    return sampleTable ? sampleTable->getSampleDescription(index) : nullptr;
}

Cardinal Track::getSampleDescriptionCount() {
    return sampleTable ? sampleTable->getSampleDescriptionCount() : 0;
}

UI32 Track::getId() const {
    return trakAtom->getId();
}

void Track::setId(UI32 trackId) {
    trakAtom->setId(trackId);
}

void Track::setMovieTimeScale(UI32 timeScale) {
    if (movieTimeScale == 0) {
        return;
    }
    trakAtom->setDuration(convertTime(trakAtom->getDuration(), movieTimeScale, timeScale));
    movieTimeScale = timeScale;
}

UI32 Track::getMovieTimeScale() const { return movieTimeScale; }

UI32 Track::getMediaTimeScale() const {
    return trakAtom ? trakAtom->getMediaTimeScale() : 0;
}

UI64 Track::getMediaDuration() const {
    return trakAtom ? trakAtom->getMediaDuration() : 0;
}

const char *Track::getTrackLanguage() const {
    auto mdhd = DYNAMIC_CAST(Mdhd, trakAtom->findChild("mdia/mdhd"));
    if (mdhd) {
        return mdhd->getLanguage().getChars();
    }
    return nullptr;
}

Result Track::attach(Moov *moovAtom) {
    if (!trakAtomIsOwned) {
        return ERROR_INTERNAL;
    }
    moovAtom->addChild(trakAtom);
    trakAtomIsOwned = false;
    return SUCCESS;
}

}
