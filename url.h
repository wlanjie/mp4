//
// Created by wlanjie on 2018/2/8.
//

#ifndef MP4_URLATOM_H
#define MP4_URLATOM_H

#include "atom.h"

namespace mp4 {

class Url : public Atom {
public:
    // class methods
    static Url *create(Size size, ByteStream &stream);

    Url();

    virtual Result writeFields(ByteStream &stream);

private:
    // methods
    Url(UI32 size,
        UI08 version,
        UI32 flags,
        ByteStream &stream);

    // members
    String m_Url;
};

}
#endif //MP4_URLATOM_H
