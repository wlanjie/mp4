//
// Created by wlanjie on 2018/2/8.
//

#include "url.h"

namespace mp4 {

Url *Url::create(Size size, ByteStream &stream) {
    UI08 version;
    UI32 flags;
    if (size < FULL_ATOM_HEADER_SIZE) {
        return nullptr;
    }
    if (FAILED(Atom::readFullHeader(stream, version, flags))) {
        return nullptr;
    }
    if (version != 0) {
        return nullptr;
    }
    return new Url(size, version, flags, stream);
}

Url::Url() : Atom(ATOM_TYPE_URL, FULL_ATOM_HEADER_SIZE, 0, 1) {
}

Url::Url(UI32 size, UI08 version, UI32 flags, ByteStream &stream) : Atom(ATOM_TYPE_URL, size, version, flags) {
    if ((this->flags & 1) == 0) {
        // not self contained
        Size str_size = size - FULL_ATOM_HEADER_SIZE;
        if (str_size > 0) {
            char *str = new char[str_size];
            stream.read(str, str_size);
            str[str_size - 1] = '\0'; // force null-termination
            m_Url = str;
            delete[] str;
        }
    }
}

Result Url::writeFields(ByteStream &stream) {
    if (flags & 1) {
        // local ref (self contained)
        return SUCCESS;
    } else {
        // url (not self contained)
        if (size32 > FULL_ATOM_HEADER_SIZE) {
            Result result = stream.write(m_Url.GetChars(), m_Url.GetLength() + 1);
            if (FAILED(result)) return result;

            // pad with zeros if necessary
            Size padding = size32 - (FULL_ATOM_HEADER_SIZE + m_Url.GetLength() + 1);
            while (padding--) stream.writeUI08(0);
        }
        return SUCCESS;
    }
}

}
