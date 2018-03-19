//
// Created by wlanjie on 2018/2/7.
//

#include "mp4.h"

class TrackFinderById : public List<Track>::Item::Finder {
public:
    TrackFinderById(UI32 trackId) : trackId(trackId) {}
    Result Test(Track* track) const {
        return track->getId() == trackId ? SUCCESS : FAILURE;
    }
private:
    UI32 trackId;
};

class TrackFinderByType : public List<Track>::Item::Finder {
public:
    TrackFinderByType(Track::Type type, Ordinal index = 0) : type(type), index(index) {}
    Result Test(Track* track) const {
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

Mp4::Mp4(UI32 timeScale, UI64 duration) :
        ftyp(nullptr) {
    moov = new Moov();
    mvhd = new Mvhd(0, 0, timeScale, duration, 0x00010000, 0x0100);

    moov->addChild(mvhd);
}

Mp4::Mp4(Moov *moov, ByteStream &stream, bool transferMoovOwnerShip) :
        moov(moov),
        moovAtomIsOwned(transferMoovOwnerShip) {
    if (moov == nullptr) {
        return;
    }
    UI32 timeScale;
    mvhd = DYNAMIC_CAST(Mvhd, moov->getChild(ATOM_TYPE_MVHD));
    if (mvhd) {
        timeScale = mvhd->getTimeScale();
    } else {
        timeScale = 0;
    }
    List<Trak>* traks = &moov->getTrakAtoms();
    List<Trak>::Item* item = traks->FirstItem();
    while (item) {
        auto* track = new Track(*item->GetData(), stream, timeScale);
        tracks.Add(track);
        item = item->GetNext();
    }
}

Mp4::~Mp4() {
    tracks.DeleteReferences();
    delete moov;
}

Result Mp4::setFileType(ByteStream& stream, UI32 majorBrand, UI32 minorVersion, UI32 *compatibleBrands, Cardinal compatibleBrandCount) {
    ftyp = new Ftyp(majorBrand, minorVersion, compatibleBrands, compatibleBrandCount);
    ftyp->write(stream);
    return 0;
}

Result Mp4::addMoov(ByteStream& stream) {
    return moov->write(stream);
}

Result Mp4::addTrack(Track* track) {
    if (track->getId() == 0) {
    }
    if (mvhd->getTimeScale() == 0) {
        mvhd->setTimeScale(track->getMediaTimeScale());
    }
    track->setMovieTimeScale(mvhd->getTimeScale());
    if (mvhd->getDuration() < track->getDuration()) {
        mvhd->setDuration(track->getDuration());
    }
    track->attach(moov);
    return SUCCESS;
}

Track *Mp4::getTrack(UI32 trackId) {
    Track* track = nullptr;
    if (SUCCEEDED(tracks.Find(TrackFinderById(trackId), track))) {
        return track;
    }
    return nullptr;
}

Track *Mp4::getTrack(Track::Type type, Ordinal index) {
    return nullptr;
}