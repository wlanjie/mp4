//
// Created by wlanjie on 2018/2/7.
//

#ifndef MP4_ATOM_H
#define MP4_ATOM_H

#include "defined.h"
#include "types.h"
#include "bytestream.h"
#include "list.h"

namespace mp4 {

const UI32 ATOM_HEADER_SIZE = 8;
const UI32 ATOM_HEADER_SIZE_64 = 16;
const UI32 FULL_ATOM_HEADER_SIZE = 12;
const UI32 FULL_ATOM_HEADER_SIZE_64 = 20;
const UI32 ATOM_MAX_NAME_SIZE = 256;
const UI32 ATOM_MAX_URI_SIZE = 512;

#define ATOM_TYPE(c1, c2, c3, c4)       \
   (((static_cast<UI32>(c1))<<24) |  \
    ((static_cast<UI32>(c2))<<16) |  \
    ((static_cast<UI32>(c3))<< 8) |  \
    ((static_cast<UI32>(c4))    ))

class AtomParent;

class Atom {
public:

    typedef UI32 Type;

    static Type typeFromString(const char *fourCC);

    static Result readFullHeader(ByteStream &stream, UI08 &version, UI32 &flags);

    explicit Atom(Type type, UI32 size = ATOM_HEADER_SIZE);

    explicit Atom(Type type, UI64 size, bool force64 = false);

    explicit Atom(Type type, UI32 size, UI08 version, UI32 flags);

    explicit Atom(Type type, UI64 size, bool force64, UI08 version, UI32 flags);

    virtual ~Atom() = default;

    UI32 getFlags() const { return flags; }

    void setFlags(UI32 flags) { this->flags = flags; }

    Type getType() const { return type; }

    void setType(Type type) { this->type = type; }

    virtual Size getHeaderSize() const;

    UI64 getSize() const { return size32 == 1 ? size64 : size32; }

    void setSize(UI64 size, bool force_64 = false);

    UI32 getSize32() const { return size32; }

    void setSize32(UI32 size) { size32 = size; }

    UI64 getSize64() const { return size64; }

    void setSize64(UI64 size) { size64 = size; }

    virtual Result write(ByteStream &stream);

    virtual Result writeHeader(ByteStream &stream);

    virtual Result writeFields(ByteStream &stream) = 0;

    // parent/child relationship methods
    virtual Result setParent(AtomParent *parent) {
        parent = parent;
        return SUCCESS;
    }

    virtual AtomParent *getParent() { return parent; }

    virtual Result detach();

    virtual Atom *clone();

protected:
    Type type;
    UI32 size32;
    UI64 size64;
    bool isFull;
    UI08 version;
    UI32 flags;
    AtomParent *parent;
};

class AtomParent {
public:

    // base methods
    virtual ~AtomParent() = default;;

    List<Atom> &getChildren() { return children; }

    Result copyChildren(AtomParent &destination) const;

    virtual Result addChild(Atom *child, int position = -1);

    virtual Result removeChild(Atom *child);

    virtual Result deleteChild(Atom::Type type, Ordinal index = 0);

    virtual Atom *getChild(Atom::Type type, Ordinal index = 0) const;

    virtual Atom *getChild(const UI08 *uuid, Ordinal index = 0) const;

    virtual Atom *findChild(const char *path,
                            bool auto_create = false,
                            bool auto_create_full = false);

    // methods designed to be overridden
    virtual void onChildChanged(Atom * /* child */) {}

    virtual void onChildAdded(Atom * /* child */) {}

    virtual void onChildRemoved(Atom * /* child */) {}

protected:
    // members
    List<Atom> children;

};

class AtomFinder : public List<Atom>::Item::Finder {
public:
    AtomFinder(Atom::Type type, Ordinal index = 0) : type(type), index(index) {}

    Result test(Atom *atom) const {
        if (atom->getType() == type) {
            if (index-- == 0) {
                return SUCCESS;
            } else {
                return FAILURE;
            }
        } else {
            return FAILURE;
        }
    }

private:
    Atom::Type type;
    mutable Ordinal index;
};

class AtomSizeAdder : public List<Atom>::Item::Operator {
public:
    AtomSizeAdder(UI64 &size) : size(size) {}

private:
    Result action(Atom *atom) const {
        size += atom->getSize();
        return SUCCESS;
    }

    UI64 &size;
};

class AtomListWriter : public List<Atom>::Item::Operator {
public:
    AtomListWriter(ByteStream &stream) :
            stream(stream) {}

    Result action(Atom *atom) const;

private:
    ByteStream &stream;
};

const Atom::Type ATOM_TYPE_UDTA = ATOM_TYPE('u', 'd', 't', 'a');
const Atom::Type ATOM_TYPE_URL = ATOM_TYPE('u', 'r', 'l', ' ');
const Atom::Type ATOM_TYPE_TRAK = ATOM_TYPE('t', 'r', 'a', 'k');
const Atom::Type ATOM_TYPE_TRAF = ATOM_TYPE('t', 'r', 'a', 'f');
const Atom::Type ATOM_TYPE_TKHD = ATOM_TYPE('t', 'k', 'h', 'd');
const Atom::Type ATOM_TYPE_STTS = ATOM_TYPE('s', 't', 't', 's');
const Atom::Type ATOM_TYPE_STSZ = ATOM_TYPE('s', 't', 's', 'z');
const Atom::Type ATOM_TYPE_STZ2 = ATOM_TYPE('s', 't', 'z', '2');
const Atom::Type ATOM_TYPE_STSS = ATOM_TYPE('s', 't', 's', 's');
const Atom::Type ATOM_TYPE_STSD = ATOM_TYPE('s', 't', 's', 'd');
const Atom::Type ATOM_TYPE_STSC = ATOM_TYPE('s', 't', 's', 'c');
const Atom::Type ATOM_TYPE_STCO = ATOM_TYPE('s', 't', 'c', 'o');
const Atom::Type ATOM_TYPE_CO64 = ATOM_TYPE('c', 'o', '6', '4');
const Atom::Type ATOM_TYPE_STBL = ATOM_TYPE('s', 't', 'b', 'l');
const Atom::Type ATOM_TYPE_SINF = ATOM_TYPE('s', 'i', 'n', 'f');
const Atom::Type ATOM_TYPE_SCHI = ATOM_TYPE('s', 'c', 'h', 'i');
const Atom::Type ATOM_TYPE_MVHD = ATOM_TYPE('m', 'v', 'h', 'd');
const Atom::Type ATOM_TYPE_MP4A = ATOM_TYPE('m', 'p', '4', 'a');
const Atom::Type ATOM_TYPE_AVC1 = ATOM_TYPE('a', 'v', 'c', '1');
const Atom::Type ATOM_TYPE_AVC2 = ATOM_TYPE('a', 'v', 'c', '2');
const Atom::Type ATOM_TYPE_AVC3 = ATOM_TYPE('a', 'v', 'c', '3');
const Atom::Type ATOM_TYPE_AVC4 = ATOM_TYPE('a', 'v', 'c', '4');
const Atom::Type ATOM_TYPE_DVAV = ATOM_TYPE('d', 'v', 'a', 'v');
const Atom::Type ATOM_TYPE_DVA1 = ATOM_TYPE('d', 'v', 'a', '1');
const Atom::Type ATOM_TYPE_HEV1 = ATOM_TYPE('h', 'e', 'v', '1');
const Atom::Type ATOM_TYPE_HVC1 = ATOM_TYPE('h', 'v', 'c', '1');
const Atom::Type ATOM_TYPE_DVHE = ATOM_TYPE('d', 'v', 'h', 'e');
const Atom::Type ATOM_TYPE_DVH1 = ATOM_TYPE('d', 'v', 'h', '1');
const Atom::Type ATOM_TYPE_ALAC = ATOM_TYPE('a', 'l', 'a', 'c');
const Atom::Type ATOM_TYPE_MOOV = ATOM_TYPE('m', 'o', 'o', 'v');
const Atom::Type ATOM_TYPE_MOOF = ATOM_TYPE('m', 'o', 'o', 'f');
const Atom::Type ATOM_TYPE_MVEX = ATOM_TYPE('m', 'v', 'e', 'x');
const Atom::Type ATOM_TYPE_MINF = ATOM_TYPE('m', 'i', 'n', 'f');
const Atom::Type ATOM_TYPE_META = ATOM_TYPE('m', 'e', 't', 'a');
const Atom::Type ATOM_TYPE_MDHD = ATOM_TYPE('m', 'd', 'h', 'd');
const Atom::Type ATOM_TYPE_ILST = ATOM_TYPE('i', 'l', 's', 't');
const Atom::Type ATOM_TYPE_HDLR = ATOM_TYPE('h', 'd', 'l', 'r');
const Atom::Type ATOM_TYPE_FTYP = ATOM_TYPE('f', 't', 'y', 'p');
const Atom::Type ATOM_TYPE_ESDS = ATOM_TYPE('e', 's', 'd', 's');
const Atom::Type ATOM_TYPE_EDTS = ATOM_TYPE('e', 'd', 't', 's');
const Atom::Type ATOM_TYPE_DREF = ATOM_TYPE('d', 'r', 'e', 'f');
const Atom::Type ATOM_TYPE_DINF = ATOM_TYPE('d', 'i', 'n', 'f');
const Atom::Type ATOM_TYPE_CTTS = ATOM_TYPE('c', 't', 't', 's');
const Atom::Type ATOM_TYPE_MDIA = ATOM_TYPE('m', 'd', 'i', 'a');
const Atom::Type ATOM_TYPE_VMHD = ATOM_TYPE('v', 'm', 'h', 'd');
const Atom::Type ATOM_TYPE_SMHD = ATOM_TYPE('s', 'm', 'h', 'd');
const Atom::Type ATOM_TYPE_MDAT = ATOM_TYPE('m', 'd', 'a', 't');
const Atom::Type ATOM_TYPE_FREE = ATOM_TYPE('f', 'r', 'e', 'e');
const Atom::Type ATOM_TYPE_HNTI = ATOM_TYPE('h', 'n', 't', 'i');
const Atom::Type ATOM_TYPE_TREF = ATOM_TYPE('t', 'r', 'e', 'f');
const Atom::Type ATOM_TYPE_ODRM = ATOM_TYPE('o', 'd', 'r', 'm');
const Atom::Type ATOM_TYPE_ODKM = ATOM_TYPE('o', 'd', 'k', 'm');
const Atom::Type ATOM_TYPE_MDRI = ATOM_TYPE('m', 'd', 'r', 'i');
const Atom::Type ATOM_TYPE_AVCC = ATOM_TYPE('a', 'v', 'c', 'C');
const Atom::Type ATOM_TYPE_HVCC = ATOM_TYPE('h', 'v', 'c', 'C');
const Atom::Type ATOM_TYPE_HVCE = ATOM_TYPE('h', 'v', 'c', 'E');
const Atom::Type ATOM_TYPE_AVCE = ATOM_TYPE('a', 'v', 'c', 'E');
const Atom::Type ATOM_TYPE_WAVE = ATOM_TYPE('w', 'a', 'v', 'e');
const Atom::Type ATOM_TYPE_WIDE = ATOM_TYPE('w', 'i', 'd', 'e');
const Atom::Type ATOM_TYPE_AC_3 = ATOM_TYPE('a', 'c', '-', '3');
const Atom::Type ATOM_TYPE_EC_3 = ATOM_TYPE('e', 'c', '-', '3');
const Atom::Type ATOM_TYPE_DTSC = ATOM_TYPE('d', 't', 's', 'c');
const Atom::Type ATOM_TYPE_DTSH = ATOM_TYPE('d', 't', 's', 'h');
const Atom::Type ATOM_TYPE_DTSL = ATOM_TYPE('d', 't', 's', 'l');
const Atom::Type ATOM_TYPE_DTSE = ATOM_TYPE('d', 't', 's', 'e');
const Atom::Type ATOM_TYPE_MFRA = ATOM_TYPE('m', 'f', 'r', 'a');
const Atom::Type ATOM_TYPE_MARL = ATOM_TYPE('m', 'a', 'r', 'l');

}
#endif //MP4_ATOM_H
