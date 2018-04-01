//
// Created by wlanjie on 2018/2/7.
//

#include "container.h"
#include "atomfactory.h"

namespace mp4 {

Container *Container::Create(Type type,
                             UI64 size,
                             bool isFull,
                             bool force64,
                             ByteStream &stream,
                             AtomFactory &factory) {
    if (isFull) {
        UI08 version;
        UI32 flags;
        if (size < FULL_ATOM_HEADER_SIZE) return NULL;
        if (FAILED(Atom::readFullHeader(stream, version, flags))) return NULL;

        // special case for 'meta' atoms, because Apple sometimes creates them as
        // regular (non-full) atoms. This is bogus, but we can try to detect it
        if (type == ATOM_TYPE_META) {
            UI32 phantom_size = (version << 24) | flags;
            if (phantom_size >= 8 && size >= 16) {
                // version+flags looks like a size. read the next 4 bytes just
                // to be sure it is a hdlr atom
                UI32 peek;
                if (FAILED(stream.readUI32(peek))) return NULL;
                if (peek == ATOM_TYPE_HDLR) {
                    // rewind the stream by 8 bytes
                    Position position;
                    stream.tell(position);
                    stream.seek(position - 8);

                    // create a non-full container
                    return new Container(type, size, force64, stream, factory);
                } else {
                    // rewind the stream by 4 bytes
                    Position position;
                    stream.tell(position);
                    stream.seek(position - 4);
                }
            }
        }

        return new Container(type, size, force64, version, flags, stream, factory);
    } else {
        return new Container(type, size, force64, stream, factory);
    }
}

Container::Container(Type type) :
        Atom(type, ATOM_HEADER_SIZE) {
}

Container::Container(Type type, UI08 version, UI32 flags) :
        Atom(type, FULL_ATOM_HEADER_SIZE, version, flags) {
}

Container::Container(Type type, UI64 size, bool force64) :
        Atom(type, size, force64) {
}

Container::Container(Type type,
                     UI64 size,
                     bool force64,
                     UI08 version,
                     UI32 flags) :
        Atom(type, size, force64, version, flags) {
}

Container::Container(Type type,
                     UI64 size,
                     bool force64,
                     ByteStream &stream,
                     AtomFactory &factory) :
        Atom(type, size, force64) {
    readChildren(factory, stream, size - getHeaderSize());
}

Container::Container(Type type,
                     UI64 size,
                     bool force64,
                     UI08 version,
                     UI32 flags,
                     ByteStream &stream,
                     AtomFactory &factory) :
        Atom(type, size, force64, version, flags) {
    readChildren(factory, stream, size - getHeaderSize());
}

Atom *
Container::clone() {
    Container *clone;
    if (isFull) {
        clone = new Container(type, version, flags);
    } else {
        clone = new Container(type);
    }

    List<Atom>::Item *child_item = children.firstItem();
    while (child_item) {
        Atom *child_clone = child_item->getData()->clone();
        if (child_clone) clone->addChild(child_clone);
        child_item = child_item->getNext();
    }

    return clone;
}

void
Container::readChildren(AtomFactory &atom_factory,
                            ByteStream &stream,
                            UI64 size) {
    Atom *atom;
    LargeSize bytes_available = size;

    // save and switch the factory's context
    atom_factory.pushContext(type);

    while (SUCCEEDED(atom_factory.createAtomFromStream(stream, bytes_available, atom))) {
        atom->setParent(this);
        children.add(atom);
    }

    // restore the saved context
    atom_factory.popContext();
}

Result Container::writeFields(ByteStream &stream) {
    // write all children
    return children.apply(AtomListWriter(stream));
}

void Container::onChildChanged(Atom *) {
    // remcompute our size
    UI64 size = getHeaderSize();
    children.apply(AtomSizeAdder(size));
    setSize(size);

    // update our parent
    if (parent) parent->onChildChanged(this);
}

void Container::onChildAdded(Atom *child) {
    // update our size
    setSize(getSize() + child->getSize());

    // update our parent
    if (parent) parent->onChildChanged(this);
}

void Container::onChildRemoved(Atom *child) {
    // update our size
    setSize(getSize() - child->getSize());

    // update our parent
    if (parent) parent->onChildChanged(this);
}

}
