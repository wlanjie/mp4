//
// Created by wlanjie on 2018/2/7.
//

#ifndef MP4_MP4_H
#define MP4_MP4_H

#include "types.h"
#include "atom.h"
#include "moov.h"
#include "ftyp.h"
#include "mvhd.h"
#include "track.h"

namespace mp4 {

class Mp4 {
public:
    Mp4(UI32 timeScale = 0, UI64 duration = 0);
    Mp4(Moov* moov, ByteStream& stream, bool transferMoovOwnerShip = true);
    ~Mp4();
    Result setFileType(ByteStream& strema, UI32 majorBrand, UI32 minorVersion, UI32* compatibleBrands, Cardinal compatibleBrandCount);
    Result addMoov(ByteStream& stream);
    Result addTrack(Track* track);

    Moov* getMoov() { return moov; }
    Track* getTrack(UI32 trackId);
    Track* getTrack(Track::Type type, Ordinal index = 0);
    List<Track>& getTracks() { return tracks; }

private:
    bool moovAtomIsOwned;
    Ftyp* ftyp;
    Moov* moov;
    Mvhd* mvhd;
    List<Track> tracks;
};

}
#endif //MP4_MP4_H
