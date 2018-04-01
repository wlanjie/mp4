//
// Created by wlanjie on 2018/3/14.
//

#include "movie.h"

namespace mp4 {

class TrackFinderById : public List<Track>::Item::Finder {
public:
    TrackFinderById(UI32 trackId) : trackId(trackId) {}
    Result test(Track* track) const {
        return track->getId() == trackId ? SUCCESS : FAILURE;
    }
private:
    UI32 trackId;
};

class TrackFinderByType : public List<Track>::Item::Finder {
public:
    TrackFinderByType(Track::Type type, Ordinal index = 0) : type(type), index(index) {}
    Result test(Track* track) const {
        if (track->getType() == type && index-- == 0)  {
            return SUCCESS;
        } else {
            return FAILURE;
        }
    }
private:
    Track::Type type;
    mutable Ordinal index;
};

Movie::Movie(UI32 timeScale, UI64 duration) : moovIsOwned(true) {
    moov = new Moov();
    mvhd = new Mvhd(0, 0, timeScale, duration, 0x00010000, 0x0100);
    moov->addChild(mvhd);
}

Movie::Movie(Moov *moov, ByteStream &stream, bool transferMoovOwnerShip) :
        moov(moov),
        moovIsOwned(transferMoovOwnerShip) {
    if (!moov) {
        return;
    }
    mvhd = DYNAMIC_CAST(Mvhd, moov->getChild(ATOM_TYPE_MVHD));
    UI32 timeScale = mvhd ? mvhd->getTimeScale() : 0;
    List<Trak>* traks = &moov->getTrakAtoms();
    List<Trak>::Item* item = traks->firstItem();
    while (item) {
        auto* track = new Track(*item->getData(), stream, timeScale);
        tracks.add(track);
        item = item->getNext();
    }
}

Movie::~Movie() {
    tracks.deleteReferences();
    if (moovIsOwned) {
        delete moov;
    }
}

Track *Movie::getTrack(UI32 trackId) {
    Track* track = nullptr;
    auto result = tracks.find(TrackFinderById(trackId), track);
    return track;
}

Track *Movie::getTrack(Track::Type type, Ordinal index) {
    Track* track = nullptr;
    tracks.find(TrackFinderByType(type, index), track);
    return track;
}

Result Movie::addTrack(Track *track) {
    if (track->getId() == 0) {
        track->setId(tracks.itemCount() + 1);
    }
    if (mvhd->getTimeScale() == 0) {
        mvhd->setTimeScale(track->getMediaTimeScale());
    }
    track->setMovieTimeScale(mvhd->getTimeScale());
    if (mvhd->getDuration() < track->getDuration()) {
        mvhd->setDuration(track->getDuration());
    }
    track->attach(moov);
    tracks.add(track);
    return SUCCESS;
}

UI32 Movie::getTimeScale() {
    return mvhd ? mvhd->getTimeScale() : 0;
}

UI64 Movie::getDuration() {
    return mvhd ? mvhd->getDuration() : 0;
}

UI32 Movie::getDurationMs() {
    return mvhd ? mvhd->getDurationMs() : 0;
}

bool Movie::hasFragments() {
    if (!moov) {
        return false;
    }
    return moov->getChild(ATOM_TYPE_MVEX) != nullptr;
}

}
