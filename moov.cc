//
// Created by wlanjie on 2018/3/8.
//

#include "moov.h"

namespace mp4 {

class TrakCollector : public List<Atom>::Item::Operator {
public:
    TrakCollector(List<Trak>* traks) : traks(traks) {}
    Result action(Atom* atom) const {
        if (atom->getType() == ATOM_TYPE_TRAK) {
            Trak* trak = DYNAMIC_CAST(Trak, atom);
            if (trak) {
                traks->add(trak);
            }
        }
        return SUCCESS;
    }

private:
    List<Trak>* traks;
};

Moov::Moov() :
        Container(ATOM_TYPE_MOOV),
        timeScale(0) {
    timeScale = 0;
}

Moov::Moov(Size size, ByteStream &stream, AtomFactory &factory) :
        Container(ATOM_TYPE_MOOV, size, false, stream, factory),
        timeScale(0) {
    children.apply(TrakCollector(&traks));
}

Result Moov::adjustChunkOffsets(SI64 offset) {
    for (List<Trak>::Item* item = traks.firstItem(); item; item = item->getNext()) {
        Trak* trak = item->getData();
        trak->adjustChunkOffsets(offset);
    }
    return SUCCESS;
}

void Moov::onChildAdded(Atom *atom) {
    if (atom->getType() == ATOM_TYPE_TRAK) {
        Trak* trak = DYNAMIC_CAST(Trak, atom);
        if (trak) {
            traks.add(trak);
        }
    }
    Container::onChildAdded(atom);
}

void Moov::onChildRemoved(Atom *atom) {
    if (atom->getType() == ATOM_TYPE_TRAK) {
        Trak* trak = DYNAMIC_CAST(Trak, atom);
        if (trak) {
            traks.remove(trak);
        }
    }
    Container::onChildRemoved(atom);
}

}
