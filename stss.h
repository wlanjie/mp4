//
// Created by wlanjie on 2018/3/11.
//

#ifndef MP4_STSS_H
#define MP4_STSS_H

#include "array.h"
#include "atom.h"

namespace mp4 {

/**
 * Sync Sample Box
 */
class Stss : public Atom {
public:
    static Stss* create(Size size, ByteStream& stream);
    Stss();
    Array<UI32>& getEntries() { return entries; }
    Result addEntry(UI32 sample);
    virtual Result writeFields(ByteStream& stream);
    virtual bool isSampleSync(Ordinal sample);
private:
    Stss(UI32 size, UI08 version, UI32 flags, ByteStream& stream);
    Array<UI32> entries;
    Ordinal LooupCache;
};

}
#endif //MP4_STSS_H
