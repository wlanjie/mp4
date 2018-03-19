//
// Created by wlanjie on 2018/3/15.
//

#ifndef MP4_ESDS_H
#define MP4_ESDS_H


#include "atom.h"
#include "descriptor.h"
#include "es_descriptor.h"

class Esds : public Atom {
public:
    static Esds* create(Size size, ByteStream& stream);
    Esds(EsDescriptor* descriptor);
    ~Esds();
    virtual Result writeFields(ByteStream& stream);
    const EsDescriptor* getEsDescriptor() const { return descriptor; }
private:
    Esds(UI32 size , UI08 version, UI32 flags, ByteStream& stream);
    EsDescriptor* descriptor;

};


#endif //MP4_ESDS_H
