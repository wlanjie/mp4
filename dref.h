//
// Created by wlanjie on 2018/3/9.
//

#ifndef MP4_DREF_H
#define MP4_DREF_H

#include "atomfactory.h"
#include "container.h"

namespace mp4 {

/**
 * data reference box, declares source(s) of media data in track
 */
class Dref : public Container {
public:
    static Dref* create(Size size, ByteStream& stream, AtomFactory& factory);

    Dref(Atom** refs, Cardinal refsCount);
    virtual Result writeFields(ByteStream& stream);
private:
    Dref(UI32 size, UI08 version, UI32 flags, ByteStream& stream, AtomFactory& factory);
};

}
#endif //MP4_DREF_H
