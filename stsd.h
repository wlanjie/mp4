//
// Created by wlanjie on 2018/3/10.
//

#ifndef MP4_STSD_H
#define MP4_STSD_H

#include "container.h"
#include "sampletable.h"
#include "sample_entry.h"

namespace mp4 {

class SampleTable;
/**
 * sample description atom
 * sample descriptions (codec types, initialization etc.)
 */
class Stsd : public Container {
public:
    static Stsd* create(Size size, ByteStream& stream, AtomFactory& factory);
    Stsd(SampleTable* sampleTable);
    ~Stsd();
    virtual Result writeFields(ByteStream& stream);
    Cardinal getSampleDescriptionCount();
    virtual SampleDescription* getSampleDescription(Ordinal index);
    virtual SampleEntry* getSampleEntry(Ordinal index);

    void onChildChanged(Atom* child);

private:
    Stsd(UI32 size, UI08 version, UI32 flags, ByteStream& stream, AtomFactory& factory);
    Array<SampleDescription*> sampleDescriptions;
};

}
#endif //MP4_STSD_H
