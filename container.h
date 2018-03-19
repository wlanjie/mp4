//
// Created by wlanjie on 2018/2/7.
//

#ifndef MP4_CONTAINERATOM_H
#define MP4_CONTAINERATOM_H

#include "atom.h"
#include "atomfactory.h"

class Container : public Atom, public AtomParent {
public:
    // class methods
    static Container *Create(Type type,
                             UI64 size,
                             bool isFull,
                             bool force64,
                             ByteStream &stream,
                             AtomFactory &factory);

    // methods
    explicit Container(Type type);

    explicit Container(Type type, UI08 version, UI32 flags);

    explicit Container(Type type, UI64 size, bool force64);

    explicit Container(Type type, UI64 size, bool force64, UI08 version, UI32 flags);

    Result writeFields(ByteStream &stream) override;

    Atom *clone() override;

    // AtomParent methods
    void onChildChanged(Atom *child) override;

    void onChildAdded(Atom *child) override;

    void onChildRemoved(Atom *child) override;

protected:
    // constructors
    Container(Type type,
              UI64 size,
              bool force64,
              UI08 version,
              UI32 flags,
              ByteStream &stream,
              AtomFactory &factory);

    Container(Type type,
              UI64 size,
              bool force64,
              ByteStream &stream,
              AtomFactory &factory);

    // methods
    void readChildren(AtomFactory &atom_factory,
                      ByteStream &stream,
                      UI64 size);
};


#endif //MP4_CONTAINERATOM_H
