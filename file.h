//
// Created by wlanjie on 2018/2/7.
//

#ifndef MP4_FILE_H
#define MP4_FILE_H

#include "atom.h"
#include "atomfactory.h"
#include "ftyp.h"
#include "movie.h"

namespace mp4 {

const UI32 FILE_BRAND_ISOM = ATOM_TYPE('i', 's', 'o', 'm');
const UI32 FILE_BRAND_ISO5 = ATOM_TYPE('i', 's', 'o', '5');
const UI32 FILE_BRAND_ISO6 = ATOM_TYPE('i', 's', 'o', '6');
const UI32 FILE_BRAND_MP41 = ATOM_TYPE('m', 'p', '4', '1');
const UI32 FILE_BRAND_MP42 = ATOM_TYPE('m', 'p', '4', '2');
const UI32 FILE_BRAND_M4A_ = ATOM_TYPE('M', '4', 'A', ' ');
const UI32 FILE_BRAND_AVC1 = ATOM_TYPE('a', 'v', 'c', '1');
const UI32 FILE_BRAND_HVC1 = ATOM_TYPE('h', 'v', 'c', '1');

class File : public AtomParent {
public:
    File(Movie* movie = nullptr);
    File(ByteStream& stream,
         AtomFactory& factory,
         bool moovOnly);
    File(ByteStream& stream, bool moovOnly = false);
    virtual ~File();
    List<Atom> &getTopLevelAtoms() { return children; }
    Movie* getMovie() { return movie; }
    Ftyp *getFileType() { return ftyp; }
    Result setFileType(UI32 major_brand,
                       UI32 minor_version,
                       UI32 *compatible_brands = nullptr,
                       Cardinal compatible_brand_count = 0);
    bool isMoovBeforeMdat() const { return moovIsBeforeMdat; }
    Result write(ByteStream& stream);
private:
    // methods
    void parseStream(ByteStream &stream,
                     AtomFactory &factory,
                     bool moovOnly);
    void release(Array<Array<UI64>*> trakChunkOffsetsBackup);

    Movie* movie;
    Ftyp* ftyp;
    bool moovIsBeforeMdat;
};

}
#endif //MP4_FILE_H
