//
// Created by wlanjie on 2018/3/9.
//

#ifndef MP4_HDLRATOM_H
#define MP4_HDLRATOM_H

#include "atom.h"

namespace mp4 {

const UI32 HANDLER_TYPE_SOUN = ATOM_TYPE('s','o','u','n');
const UI32 HANDLER_TYPE_VIDE = ATOM_TYPE('v','i','d','e');

class Hdlr : public Atom {
public:
    static Hdlr* create(Size size, ByteStream& stream);

    Hdlr(UI32 hdlrType, const char* hdlrName);
    virtual Result writeFields(ByteStream& stream);
    UI32 getHandlerType() { return handlerType; }
private:
    Hdlr(UI32 size,
             UI08 version,
             UI32 flags,
             ByteStream& stream);

    UI32 handlerType;
    UI32 reserved[3];
    String handlerName;
};

}
#endif //MP4_HDLRATOM_H
