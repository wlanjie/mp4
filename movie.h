//
// Created by wlanjie on 2018/3/14.
//

#ifndef MP4_MOVIE_H
#define MP4_MOVIE_H


#include "types.h"
#include "moov.h"
#include "mvhd.h"
#include "track.h"

class Movie {
public:
    Movie(UI32 timeScale = 0, UI64 duration = 0);
    Movie(Moov* moov, ByteStream& stream, bool transferMoovOwnerShip = true);
    virtual ~Movie();

    Moov* getMoov() { return moov; }
    Mvhd* getMvhd() { return mvhd; }
    List<Track>& getTracks() { return tracks; }
    Track* getTrack(UI32 trackId);
    Track* getTrack(Track::Type type, Ordinal index = 0);
    Result addTrack(Track* track);
    UI32 getTimeScale();
    UI64 getDuration();
    UI32 getDurationMs();
    bool hasFragments();
private:
    Moov* moov;
    bool moovIsOwned;
    Mvhd* mvhd;
    List<Track> tracks;
};


#endif //MP4_MOVIE_H
