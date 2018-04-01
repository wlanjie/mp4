//
// Created by wlanjie on 2018/2/7.
//

#include "atom.h"
#include "utils.h"
#include "container.h"
#include "debug.h"

namespace mp4 {

static const unsigned int ATOM_MAX_CLONE_SIZE = 1048576; // 1 meg
static const unsigned int UNKNOWN_ATOM_MAX_LOCAL_PAYLOAD_SIZE = 4096;

Atom::Type Atom::typeFromString(const char *fourCC) {
    return ((UI32) fourCC[0]) << 24 |
           ((UI32) fourCC[1]) << 16 |
           ((UI32) fourCC[2]) << 8 |
           ((UI32) fourCC[3]);
}

Atom::Atom(Type type, UI32 size /* = ATOM_HEADER_SIZE */) :
        type(type),
        size32(size),
        size64(0),
        isFull(false),
        version(0),
        flags(0),
        parent(NULL) {
}

Atom::Atom(Type type, UI64 size, bool force_64) :
        type(type),
        size32(0),
        size64(0),
        isFull(false),
        version(0),
        flags(0),
        parent(NULL) {
    setSize(size, force_64);
}

Atom::Atom(Type type,
           UI32 size,
           UI08 version,
           UI32 flags) :
        type(type),
        size32(size),
        size64(0),
        isFull(true),
        version(version),
        flags(flags),
        parent(NULL) {
}

Atom::Atom(Type type,
           UI64 size,
           bool force_64,
           UI08 version,
           UI32 flags) :
        type(type),
        size32(0),
        size64(0),
        isFull(true),
        version(version),
        flags(flags),
        parent(NULL) {
    setSize(size, force_64);
}


Result Atom::readFullHeader(ByteStream &stream,
                            UI08 &version,
                            UI32 &flags) {
    UI32 header;
    CHECK(stream.readUI32(header));
    version = (header >> 24) & 0x000000FF;
    flags = (header) & 0x00FFFFFF;

    return SUCCESS;
}

void Atom::setSize(UI64 size, bool force_64) {
    if (!force_64) {
        // see if we need to implicitely force 64-bit encoding
        if (size32 == 1 && size64 <= 0xFFFFFFFF) {
            // we have a forced 64-bit encoding
            force_64 = true;
        }
    }
    if ((size >> 32) == 0 && !force_64) {
        size32 = (UI32) size;
        size64 = 0;
    } else {
        size32 = 1;
        size64 = size;
    }
}

Size Atom::getHeaderSize() const {
    return (isFull ? FULL_ATOM_HEADER_SIZE : ATOM_HEADER_SIZE) + (size32 == 1 ? 8 : 0);
}

Result Atom::writeHeader(ByteStream &stream) {
    Result result;

    // write the size
    result = stream.writeUI32(size32);
    if (FAILED(result)) return result;

    // write the type
    result = stream.writeUI32(type);
    if (FAILED(result)) return result;

    // handle 64-bit sizes
    if (size32 == 1) {
        result = stream.writeUI64(size64);
        if (FAILED(result)) return result;
    }

    // for full atoms, write version and flags
    if (isFull) {
        result = stream.writeUI08(version);
        if (FAILED(result)) return result;
        result = stream.writeUI24(flags);
        if (FAILED(result)) return result;
    }

    return SUCCESS;
}

Result Atom::write(ByteStream &stream) {
    Result result;

#if defined(DEBUG)
    Position before;
    stream.tell(before);
#endif

    // write the header
    result = writeHeader(stream);
    if (FAILED(result)) return result;

    // write the fields
    result = writeFields(stream);
    if (FAILED(result)) return result;

#if defined(DEBUG)
    Position after;
    stream.tell(after);
    UI64 atom_size = getSize();
    if (after-before != atom_size) {
        debug("ERROR: atom size mismatch (declared size=%d, actual size=%d)\n",
                  (UI32)atom_size, (UI32)(after-before));
        Atom* atom = this;
        while (atom) {
            char name[7];
            name[0] = '[';
            FormatFourCharsPrintable(&name[1], atom->getType());
            name[5] = ']';
            name[6] = '\0';
            debug("       while writing %s\n", name);
            atom = DYNAMIC_CAST(Atom, atom->getParent());
        }
        ASSERT(after-before == atom_size);
    }
#endif

    return SUCCESS;
}

Result Atom::detach() {
    if (parent) {
        return parent->removeChild(this);
    } else {
        return SUCCESS;
    }
}

Atom *Atom::clone() {
    Atom *clone = NULL;

    // check the size (refuse to clone atoms that are too large)
    LargeSize size = getSize();
    if (size > ATOM_MAX_CLONE_SIZE) return nullptr;

    // create a memory byte stream to which we can serialize
    MemoryByteStream *mbs = new MemoryByteStream((Size) getSize());

    // serialize to memory
    if (FAILED(write(*mbs))) {
        mbs->release();
        return NULL;
    }

    // create the clone from the serialized form
    mbs->seek(0);
    DefaultAtomFactory factory;
    factory.createAtomFromStream(*mbs, clone);

    // release the memory stream
    mbs->release();

    return clone;
}

Result AtomParent::removeChild(Atom *child) {
    // check that this is our child
    if (child->getParent() != this) return ERROR_INVALID_PARAMETERS;

    // remove the child
    Result result = children.remove(child);
    if (FAILED(result)) return result;

    // notify that child that it is orphaned
    child->setParent(NULL);

    // get a chance to update
    onChildRemoved(child);

    return SUCCESS;
}

Result AtomParent::deleteChild(Atom::Type type, Ordinal index /* = 0 */) {
    // find the child
    Atom *child = getChild(type, index);
    if (child == NULL) return FAILURE;

    // remove the child
    Result result = removeChild(child);
    if (FAILED(result)) return result;

    // delete the child
    delete child;

    return SUCCESS;
}

Atom *AtomParent::getChild(Atom::Type type, Ordinal index /* = 0 */) const {
    Atom *atom;
    Result result = children.find(AtomFinder(type, index), atom);
    if (SUCCEEDED(result)) {
        return atom;
    } else {
        return NULL;
    }
}

Atom *AtomParent::getChild(const UI08 *uuid, Ordinal index /* = 0 */) const {
    for (List<Atom>::Item *item = children.firstItem();
         item;
         item = item->getNext()) {
        Atom *atom = item->getData();
//        if (atom->getType() == ATOM_TYPE_UUID) {
//            UuidAtom *uuid_atom = DYNAMIC_CAST(UuidAtom, atom);
//            if (CompareMemory(uuid_atom->getUuid(), uuid, 16) == 0) {
//                if (index == 0) return atom;
//                --index;
//            }
//        }
    }
    return NULL;
}

Atom *AtomParent::findChild(const char *path,
                            bool auto_create,
                            bool auto_create_full) {
    // start from here
    AtomParent *parent = this;

    // walk the path
    while (path[0] && path[1] && path[2] && path[3]) {
        // we have 4 valid chars
        const char *end = &path[4];

        // look for the end or a separator
        while (*end != '\0' && *end != '/' && *end != '[') {
            ++end;
        }

        // decide if this is a 4-character code or a UUID
        UI08 uuid[16];
        Atom::Type type = 0;
        bool is_uuid = false;
        if (end == path + 4) {
            // 4-character code
            type = ATOM_TYPE(path[0], path[1], path[2], path[3]);
        } else if (end == path + 32) {
            // UUID
            is_uuid = true;
            parseHex(path, uuid, sizeof(uuid));
        } else {
            // malformed path
            return NULL;
        }

        // parse the array index, if any
        int index = 0;
        if (*end == '[') {
            const char *x = end + 1;
            while (*x >= '0' && *x <= '9') {
                index = 10 * index + (*x++ - '0');
            }
            if (*x != ']') {
                // malformed path
                return NULL;
            }
            end = x + 1;
        }

        // check what's at the end now
        if (*end == '/') {
            ++end;
        } else if (*end != '\0') {
            // malformed path
            return NULL;
        }

        // look for this atom in the current list
        Atom *atom = NULL;
        if (is_uuid) {
            atom = parent->getChild(uuid, index);
        } else {
            atom = parent->getChild(type, index);
        }
        if (atom == NULL) {
            // not found
            if (auto_create && (index == 0)) {
                if (auto_create_full) {
                    atom = new Container(type, (UI32) 0, (UI32) 0);
                } else {
                    atom = new Container(type);
                }
                parent->addChild(atom);
            } else {
                return NULL;
            }
        }

        if (*end) {
            path = end;
            // if this atom is an atom parent, recurse
            parent = DYNAMIC_CAST(Container, atom);
            if (parent == NULL) return NULL;
        } else {
            return atom;
        }
    }

    // not found
    return NULL;
}

Result AtomParent::copyChildren(AtomParent &destination) const {
    for (List<Atom>::Item *child = children.firstItem(); child; child = child->getNext()) {
        Atom *child_clone = child->getData()->clone();
        destination.addChild(child_clone);
    }

    return SUCCESS;
}

Result AtomParent::addChild(Atom *child, int position) {
    if (child->getParent() != NULL) return ERROR_INVALID_PARAMETERS;

    // attach the child
    Result result;
    if (position == -1) {
        // insert at the tail
        result = children.add(child);
    } else if (position == 0) {
        // insert at the head
        result = children.insert(NULL, child);
    } else {
        // insert after <n-1>
        List<Atom>::Item* insertion_point = children.firstItem();
        unsigned int count = position;
        while (insertion_point && --count) {
            insertion_point = insertion_point->getNext();
        }
        if (insertion_point) {
            result = children.insert(insertion_point, child);
        } else {
            result = ERROR_OUT_OF_RANGE;
        }
    }
    if (FAILED(result)) return result;

    // notify the child of its parent
    child->setParent(this);

    // get a chance to update
    onChildAdded(child);

    return SUCCESS;
}

const unsigned int ATOM_LIST_WRITER_MAX_PADDING=1024;

Result AtomListWriter::action(Atom *atom) const {
    Position before;
    stream.tell(before);

    atom->write(stream);

    Position after;
    stream.tell(after);

    UI64 bytes_written = after-before;
    ASSERT(bytes_written <= atom->getSize());
    if (bytes_written < atom->getSize()) {
        UI64 padding = atom->getSize()-bytes_written;
        if (padding > ATOM_LIST_WRITER_MAX_PADDING) {
            return FAILURE;
        } else {
            for (unsigned int i=0; i<padding; i++) {
                stream.writeUI08(0);
            }
        }
    }

    return SUCCESS;
}

}
