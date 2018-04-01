//
// Created by wlanjie on 2018/2/7.
//

#include "file.h"

namespace mp4 {

File::File(Movie* movie) :
        movie(movie),
        ftyp(nullptr),
        moovIsBeforeMdat(true) {}

File::File(ByteStream& stream,
           AtomFactory& factory,
           bool moovOnly) :
        ftyp(nullptr),
        movie(nullptr),
        moovIsBeforeMdat(true) {
    parseStream(stream, factory, moovOnly);
}

File::File(ByteStream &stream, bool moovOnly) :
        ftyp(nullptr),
        movie(nullptr),
        moovIsBeforeMdat(true) {
    DefaultAtomFactory factory;
    parseStream(stream, factory, moovOnly);
}

File::~File() {
    delete movie;
}

void File::parseStream(ByteStream &stream,
                       AtomFactory &atom_factory,
                       bool moov_only) {
    // parse top-level atoms
    Atom *atom;
    Position stream_position;
    bool keep_parsing = true;
    while (keep_parsing && SUCCEEDED(stream.tell(stream_position)) && SUCCEEDED(atom_factory.createAtomFromStream(stream, atom))) {
        addChild(atom);
        switch (atom->getType()) {
            case ATOM_TYPE_MOOV:
                movie = new Movie(DYNAMIC_CAST(Moov, atom), stream, false);
                if (moov_only) keep_parsing = false;
                break;

            case ATOM_TYPE_FTYP:
                ftyp = DYNAMIC_CAST(Ftyp, atom);
                break;

            case ATOM_TYPE_MDAT:
                // see if we are before the moov atom
                break;
        }
    }
}

Result File::setFileType(UI32 majorBrand,
                         UI32 minorVersion,
                         UI32 *compatibleBrands,
                         Cardinal compatibleBrandCount) {
    if (ftyp) {
        removeChild(ftyp);
        delete ftyp;
    }
    ftyp = new Ftyp(majorBrand,
                    minorVersion,
                    compatibleBrands,
                    compatibleBrandCount);
    addChild(ftyp, 0);
    return SUCCESS;
}

Result File::write(ByteStream &stream) {
    if (ftyp) {
        ftyp->write(stream);
    }
    for (List<Atom>::Item* item = getChildren().firstItem(); item; item = item->getNext()) {
        auto* atom = item->getData();
        if (atom->getType() != ATOM_TYPE_MDAT && atom->getType() != ATOM_TYPE_FTYP && atom->getType() != ATOM_TYPE_MOOV) {
            atom->write(stream);
        }
    }
    if (!movie) {
        return SUCCESS;
    }
    Position position;
    stream.tell(position);
    unsigned int t = 0;
    UI64 mdatSize = ATOM_HEADER_SIZE;
    UI64 mdatPosition = position + movie->getMoov()->getSize();
    Array<Array<UI64>*> trakChunkOffsetsBackup;
    Array<UI64> chunkOffsets;
    Result result = SUCCESS;
    for (List<Track>::Item* item = movie->getTracks().firstItem(); item; item = item->getNext()) {
        auto* track = item->getData();
        auto* trak = track->useTrakAtom();
        auto* chunkOffsetsBackup = new Array<UI64>();
        trakChunkOffsetsBackup.Append(chunkOffsetsBackup);
        result = trak->getChunkOffsets(*chunkOffsetsBackup);
        if (FAILED(result)) {
            release(trakChunkOffsetsBackup);
            return result;
        }
        chunkOffsets.SetItemCount(chunkOffsetsBackup->ItemCount());
        auto sampleCount = track->getSampleCount();
        auto* sampleTable = track->getSampleTable();
        Sample sample;
        for (Ordinal i = 0; i < sampleCount; i++) {
            Ordinal chunkIndex = 0;
            Ordinal positionInChunk = 0;
            sampleTable->getSampleChunkPosition(i, chunkIndex, positionInChunk);
            sampleTable->getSample(i, sample);
            if (positionInChunk == 0) {
                if (chunkIndex >= chunkOffsets.ItemCount()) {
                    release(trakChunkOffsetsBackup);
                    return ERROR_INVALID_FORMAT;
                }
                chunkOffsets[chunkIndex] = mdatPosition + mdatSize;
            }
            mdatSize += sample.getSize();
        }
        result = trak->setChunkOffsets(chunkOffsets);
    }
    // write the moov atom
    movie->getMoov()->write(stream);
    // create and write the media data (mdat)
    stream.writeUI32((UI32) mdatSize);
    stream.writeUI32(ATOM_TYPE_MDAT);

    // write all tracks and restore the chunk offsets to their backed-up values
    for (List<Track>::Item* item = movie->getTracks().firstItem(); item; item = item->getNext()) {
        auto* track = item->getData();
        auto* trak = track->useTrakAtom();
        result = trak->setChunkOffsets(*trakChunkOffsetsBackup[t]);

        // write all the track's samples
        auto sampleCount = track->getSampleCount();
        Sample sample;
        DataBuffer sampleData;
        for (Ordinal i = 0; i < sampleCount; i++) {
            track->readSample(i, sample, sampleData);
            stream.write(sampleData.getData(), sampleData.getDataSize());
        }
    }
    release(trakChunkOffsetsBackup);
    return result;
}

void File::release(Array<Array<UI64> *> trakChunkOffsetsBackup) {
    for (unsigned int i = 0; i < trakChunkOffsetsBackup.ItemCount(); i++) {
        delete trakChunkOffsetsBackup[i];
    }
}

}
