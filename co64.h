//
// Created by wlanjie on 2018/3/11.
//

#ifndef MP4_CO64_H
#define MP4_CO64_H

#include "atom.h"

namespace mp4 {
/**
 * Chunk Offset Box
 * 64-bit chunk offset
 */
class Co64 : public Atom {
public:
    static Co64* create(Size size, ByteStream& stream);
    Co64(UI64* offsets, UI32 offsetCount);
    ~Co64();
    virtual Result writeFields(ByteStream& stream);
    Cardinal getChunkCount() { return entryCount; }
    UI64* getChunkOffsets() { return entries; }
    Result getChunkOffset(Ordinal chunk , UI64& chunkOffset);
    Result setChunkOffset(Ordinal chunk, UI64 chunkOffset);
    Result adjustChunkOffset(SI64 delta);
private:
    Co64(UI32 size, UI08 version, UI32 flags, ByteStream& stream);
    UI64* entries;
    UI32 entryCount;
};

}
#endif //MP4_CO64_H
