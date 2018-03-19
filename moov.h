//
// Created by wlanjie on 2018/3/8.
//

#ifndef MP4_MOOVATOM_H
#define MP4_MOOVATOM_H

#include "container.h"
#include "trak.h"

class Moov : public Container {

public:
    static Moov* create(Size size, ByteStream& stream, AtomFactory& factory) {
        return new Moov(size, stream, factory);
    }

    Moov();

    List<Trak>& getTrakAtoms() { return traks; }
    UI32 getTimeScale() {
        return timeScale;
    }

    Result adjustChunkOffsets(SI64 offset);
    void onChildAdded(Atom* atom);
    void onChildRemoved(Atom* atom);
private:
    Moov(Size size, ByteStream& stream, AtomFactory& factory);
    List<Trak> traks;
    UI32 timeScale;
};


#endif //MP4_MOOVATOM_H
