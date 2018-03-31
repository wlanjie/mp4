//
// Created by wlanjie on 2018/3/9.
//

#ifndef MP4_SMHD_H
#define MP4_SMHD_H

#include "atom.h"

namespace mp4 {

/**
 * sound media header, overall information (sound track only)
 */
class Smhd : public Atom {
public:
    static Smhd* create(Size size, ByteStream& stream);
    Smhd(UI16 balance);
    virtual Result writeFields(ByteStream& stream);
private:
    Smhd(UI32 size, UI08 version, UI32 flags, ByteStream& stream);
    UI16 balance;
    UI16 reserved;
};

}
#endif //MP4_SMHD_H
