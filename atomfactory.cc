//
// Created by wlanjie on 2018/2/7.
//

#include "atomfactory.h"
#include "sample_entry.h"
#include "moov.h"
#include "mvhd.h"
#include "hdlr.h"
#include "stz2.h"
#include "avcc.h"
#include "ftyp.h"
#include "vmhd.h"
#include "dref.h"
#include "url.h"
#include "smhd.h"

namespace mp4 {

AtomFactory::~AtomFactory() {
    m_TypeHandlers.deleteReferences();
}

Result AtomFactory::addTypeHandler(TypeHandler *handler) {
    return m_TypeHandlers.add(handler);
}

Result AtomFactory::removeTypeHandler(TypeHandler *handler) {
    return m_TypeHandlers.remove(handler);
}

Result AtomFactory::createAtomFromStream(ByteStream &stream, LargeSize &bytesAvailable, Atom *&atom) {
    Result result;
    atom = nullptr;
    if (bytesAvailable < 8) {
        return ERROR_EOS;
    }
    Position start;
    stream.tell(start);
    UI32 size32;
    result = stream.readUI32(size32);
    if (FAILED(result)) {
        stream.seek(start);
        return result;
    }
    UI64 size = size32;
    Atom::Type type;
    result = stream.readUI32(type);
    if (FAILED(result)) {
        stream.seek(start);
        return result;
    }
    bool atomIsLarge = false;
    bool force64 = false;
    if (size == 0) {
        LargeSize streamSize = 0;
        stream.getSize(streamSize);
        if (streamSize >= start) {
            size = streamSize - start;
        }
    } else if (size == 1) {
        atomIsLarge = true;
        if (bytesAvailable < 16) {
            stream.seek(start);
            return ERROR_INVALID_FORMAT;
        }
        stream.readUI64(size);
        if (size <= 0xFFFFFFFF) {
            force64 = true;
        }
    }
    if ((size > 0 && size < 8) || size > bytesAvailable) {
        stream.seek(start);
        return ERROR_INVALID_FORMAT;
    }
    result = createAtomFromStream(stream, type, size32, size, atom);
    if (FAILED(result)) {
        return result;
    }
    if (!atom) {
        unsigned int payloadOffset = 8;
        if (atomIsLarge) {
            payloadOffset += 8;
        }
        stream.seek(start + payloadOffset);
        return ERROR_INVALID_FORMAT;
    }

    if (force64) {
        atom->setSize32(1);
        atom->setSize64(size);
    }
    bytesAvailable -= size;
    result = stream.seek(start + size);
    if (FAILED(result)) {
        atom = nullptr;
    }
    return result;
}

Result AtomFactory::createAtomFromStream(ByteStream &stream, UI32 type, UI32 size32, UI64 size64, Atom *&atom) {
    bool atomIsLarge = (size32 == 1);
    bool force64 = (size32 == 1 && ((size64 >> 32) == 0));
    if (getContext() == ATOM_TYPE_STSD) {
        if (atomIsLarge) {
            return ERROR_INVALID_FORMAT;
        }
        switch (type) {
            case ATOM_TYPE_MP4A:
                atom = new Mp4aSampleEntry(size32, stream, *this);
                break;
            case ATOM_TYPE_AVC1:
            case ATOM_TYPE_AVC2:
            case ATOM_TYPE_AVC3:
            case ATOM_TYPE_AVC4:
            case ATOM_TYPE_DVAV:
            case ATOM_TYPE_DVA1:
                atom = new AvcSampleEntry(type, size32, stream, *this);
                break;

            case ATOM_TYPE_HEV1:
            case ATOM_TYPE_HVC1:
            case ATOM_TYPE_DVHE:
            case ATOM_TYPE_DVH1:
                // h265
                break;

            case ATOM_TYPE_ALAC:
            case ATOM_TYPE_AC_3:
            case ATOM_TYPE_EC_3:
            case ATOM_TYPE_DTSC:
            case ATOM_TYPE_DTSH:
            case ATOM_TYPE_DTSL:
            case ATOM_TYPE_DTSE:
                atom = new AudioSampleEntry(type, size32, stream, *this);
                break;
        }
    } else {
        switch(type) {
            case ATOM_TYPE_FTYP:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Ftyp::create(size32, stream);
                break;

            case ATOM_TYPE_MOOV:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Moov::create(size32, stream, *this);
                break;

            case ATOM_TYPE_MVHD:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Mvhd::create(size32, stream);
                break;

            case ATOM_TYPE_TRAK:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Trak::create(size32, stream, *this);
                break;

            case ATOM_TYPE_HDLR:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Hdlr::create(size32, stream);
                break;

            case ATOM_TYPE_TKHD:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Tkhd::create(size32, stream);
                break;

            case ATOM_TYPE_MDHD:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Mdhd::create(size32, stream);
                break;

            case ATOM_TYPE_VMHD:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Vmhd::create(size32, stream);
                break;

            case ATOM_TYPE_STSD:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Stsd::create(size32, stream, *this);
                break;

            case ATOM_TYPE_STSC:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Stsc::create(size32, stream);
                break;

            case ATOM_TYPE_STCO:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Stco::create(size32, stream);
                break;

            case ATOM_TYPE_CO64:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Co64::create(size32, stream);
                break;

            case ATOM_TYPE_STSZ:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Stsz::create(size32, stream);
                break;

            case ATOM_TYPE_STZ2:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Stz2::create(size32, stream);
                break;

            case ATOM_TYPE_STTS:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Stts::create(size32, stream);
                break;

            case ATOM_TYPE_CTTS:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Ctts::create(size32, stream);
                break;

            case ATOM_TYPE_STSS:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Stss::create(size32, stream);
                break;

            case ATOM_TYPE_ESDS:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Esds::create(size32, stream);
                break;

            case ATOM_TYPE_AVCC:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Avcc::create(size32, stream);
                break;

            case ATOM_TYPE_AVCE:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Avcc::create(size32, stream);
                if (atom) {
                    atom->setType(ATOM_TYPE_AVCE);
                }
                break;

            case ATOM_TYPE_HVCC:
                // HvccAtom
                break;

            case ATOM_TYPE_HVCE:
                // HvccAtom
                break;

            case ATOM_TYPE_DREF:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Dref::create(size32, stream, *this);
                break;

            case ATOM_TYPE_URL:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Url::create(size32, stream);
                break;

            case ATOM_TYPE_SMHD:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Smhd::create(size32, stream);
                break;

                // container atoms
            case ATOM_TYPE_MOOF:
            case ATOM_TYPE_MVEX:
            case ATOM_TYPE_TRAF:
            case ATOM_TYPE_TREF:
            case ATOM_TYPE_MFRA:
            case ATOM_TYPE_HNTI:
            case ATOM_TYPE_STBL:
            case ATOM_TYPE_MDIA:
            case ATOM_TYPE_DINF:
            case ATOM_TYPE_MINF:
            case ATOM_TYPE_SCHI:
            case ATOM_TYPE_SINF:
            case ATOM_TYPE_UDTA:
            case ATOM_TYPE_ILST:
            case ATOM_TYPE_EDTS:
            case ATOM_TYPE_MDRI:
            case ATOM_TYPE_WAVE:
                if (atomIsLarge) return ERROR_INVALID_FORMAT;
                atom = Container::Create(type, size64, false, force64, stream, *this);
                break;

                // containers, only at the top
            case ATOM_TYPE_MARL:
                if (getContext() == 0) {
                    atom = Container::Create(type, size64, false, force64, stream, *this);
                }
                break;

                // full container atoms
            case ATOM_TYPE_META:
            case ATOM_TYPE_ODRM:
            case ATOM_TYPE_ODKM:
                atom = Container::Create(type, size64, true, force64, stream, *this);
                break;

            case ATOM_TYPE_FREE:
            case ATOM_TYPE_WIDE:
            case ATOM_TYPE_MDAT:
                // generic atoms
                break;

            default:
                List<TypeHandler>::Item* item = m_TypeHandlers.firstItem();
                while (item) {
                    TypeHandler* handler = item->getData();
                    if (SUCCEEDED(handler->createAtom(type, size32, stream, getContext(), atom))) {
                        break;
                    }
                    item = item->getNext();
                }
                break;

        }
    }
    return SUCCESS;
}

Result AtomFactory::createAtomFromStream(ByteStream &stream, Atom *&atom) {
    LargeSize stream_size = 0;
    Position stream_position = 0;
    LargeSize bytes_available = (LargeSize) (-1);
    if (SUCCEEDED(stream.getSize(stream_size)) &&
        stream_size != 0 &&
        SUCCEEDED(stream.tell(stream_position)) &&
        stream_position <= stream_size) {
        bytes_available = stream_size - stream_position;
    }
    return createAtomFromStream(stream, bytes_available, atom);
}

Result AtomFactory::createAtomsFromStream(ByteStream &stream, AtomParent &atoms) {
    LargeSize stream_size = 0;
    Position stream_position = 0;
    LargeSize bytes_available = (LargeSize) (-1);
    if (SUCCEEDED(stream.getSize(stream_size)) &&
        stream_size != 0 &&
        SUCCEEDED(stream.tell(stream_position)) &&
        stream_position <= stream_size) {
        bytes_available = stream_size - stream_position;
    }
    return createAtomsFromStream(stream, bytes_available, atoms);
}

Result AtomFactory::createAtomsFromStream(ByteStream &stream,
                                   LargeSize bytes_available,
                                   AtomParent &atoms) {
    Result result;
    do {
        Atom *atom = NULL;
        result = createAtomFromStream(stream, bytes_available, atom);
        if (SUCCEEDED(result) && atom != NULL) {
            atoms.addChild(atom);
        }
    } while (SUCCEEDED(result));

    return SUCCESS;
}

void AtomFactory::pushContext(Atom::Type context) {
    m_ContextStack.Append(context);
}

void AtomFactory::popContext() {
    m_ContextStack.RemoveLast();
}

Atom::Type AtomFactory::getContext(Ordinal depth) {
    Ordinal available = m_ContextStack.ItemCount();
    if (depth >= available) return 0;
    return m_ContextStack[available - depth - 1];
}

DefaultAtomFactory DefaultAtomFactory::Instance_;

DefaultAtomFactory::DefaultAtomFactory() {
    initialize();
}

Result DefaultAtomFactory::initialize() {
    // register built-in type handlers
    return SUCCESS;
}

}
