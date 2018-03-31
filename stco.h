//
// Created by wlanjie on 2018/3/11.
//

#ifndef MP4_STCO_H
#define MP4_STCO_H

#include "atom.h"

namespace mp4 {

/**
 * Chunk Offset Box
 * chunk offset, partial data-offset information
 */
class Stco : public Atom {
public:
    static Stco* create(Size size, ByteStream& stream);
    Stco(UI32* offsets, UI32 offsetCount);
    ~Stco();
    virtual Result writeFields(ByteStream& stream);
    Cardinal getChunkCount() { return entryCount; }
    UI32* getChunkOffsets() { return entries; }
    Result getChunkOffset(Ordinal chunk, UI32& chunkOffsets);
    Result setChunkOffset(Ordinal chunk, UI32 chunkOffsets);
    Result adjustChunkOffsets(int delta);

private:
    Stco(UI32 size, UI08 version, UI32 flags, ByteStream& stream);
    UI32 * entries;
    UI32 entryCount;
};

}
#endif //MP4_STCO_H
