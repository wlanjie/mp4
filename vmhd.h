//
// Created by wlanjie on 2018/3/9.
//

#ifndef MP4_VMHD_H
#define MP4_VMHD_H

#include "atom.h"

namespace mp4 {

/**
 * video media header, overall information (video track only)
 */
class Vmhd : public Atom {
public:
    static Vmhd* create(Size size, ByteStream& stream);
    Vmhd(UI16 graphicsMode, UI16 r, UI16 g, UI16 b);
    virtual Result writeFields(ByteStream& stream);
private:
    Vmhd(UI32 size, UI08 version, UI32 flags, ByteStream& stream);
    UI16 graphicsMode;
    UI16 opColor[3];
};

}
#endif //MP4_VMHD_H
