//
// Created by wlanjie on 2018/3/13.
//

#ifndef MP4_STZ2_H
#define MP4_STZ2_H


#include "atom.h"
#include "array.h"

class Stz2 : public Atom {
public:
    static Stz2* create(Size size, ByteStream& stream);
    Stz2(UI08 fieldSize);
    virtual Result writeFields(ByteStream& stream);
    UI32 getSampleCount();
    Result getSampleSize(Ordinal sample, Size& sampleSize);
    virtual Result setSampleSize(Ordinal sample, Size sampleSize);
    virtual Result addEntry(UI32 size);

private:
    Stz2(UI32 size, UI08 version, UI32 flags, ByteStream& stream);
    UI08 fieldsSize;
    UI32 sampleCount;
    Array<UI32> entries;
};


#endif //MP4_STZ2_H
