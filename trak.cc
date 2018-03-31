//
// Created by wlanjie on 2018/3/9.
//

#include "trak.h"
#include "hdlr.h"
#include "vmhd.h"
#include "smhd.h"
#include "dref.h"
#include "url.h"

namespace mp4 {

Trak::Trak(SampleTable* sampleTable,
           Atom::Type hdlrType,
           const char *hdlrName,
           UI32 trackId,
           UI32 creationTime,
           UI32 modificationTime,
           UI64 trackDuration,
           UI32 mediaTimeScale,
           UI64 mediaDuration,
           UI16 volume,
           const char *language,
           UI32 width,
           UI32 height,
           UI16 layer,
           UI16 alternateGroup,
           const SI32 *matrix) :
        Container(ATOM_TYPE_TRAK) {
    tkhdAtom = new Tkhd(creationTime,
                            modificationTime,
                            trackId,
                            trackDuration,
                            volume,
                            width,
                            height,
                            layer,
                            alternateGroup,
                            matrix);
    addChild(tkhdAtom);

    Container* mdia = new Container(ATOM_TYPE_MDIA);
    mdhdAtom = new Mdhd(creationTime, modificationTime, mediaTimeScale, mediaDuration, language);
    mdia->addChild(mdhdAtom);
    Hdlr* hdlr = new Hdlr(hdlrType, hdlrName);
    mdia->addChild(hdlr);
    Container* minf = new Container(ATOM_TYPE_MINF);

    if (hdlrType == HANDLER_TYPE_VIDE) {
        Vmhd* vmhd = new Vmhd(0, 0, 0, 0);
        minf->addChild(vmhd);
    } else if (hdlrType == HANDLER_TYPE_SOUN) {
        Smhd* smhd = new Smhd(0);
        minf->addChild(smhd);
    }

    Container* dinf = new Container(ATOM_TYPE_DINF);
    Atom* url = new Url();
    Dref* dref = new Dref(&url, 1);
    dinf->addChild(dref);
    minf->addChild(dinf);

    Container* stbl;
    Result result = sampleTable->generateStbl(stbl);
    if (SUCCEEDED(result)) {
        minf->addChild(stbl);
    }
    mdia->addChild(minf);
    addChild(mdia);
}

Trak::Trak(UI32 size, ByteStream &stream, AtomFactory &factory) :
        Container(ATOM_TYPE_TRAK, size, false, stream, factory) {
    tkhdAtom = DYNAMIC_CAST(Tkhd, findChild("tkhd"));
    mdhdAtom = DYNAMIC_CAST(Mdhd, findChild("mdia/mdhd"));
}

Result Trak::adjustChunkOffsets(SI64 delta) {
    Atom* atom;
    if ((atom = findChild("mdia/minf/stbl/stco")) != nullptr) {
        auto* stco = DYNAMIC_CAST(Stco, atom);
        return stco->adjustChunkOffsets((int) delta);
    } else if ((atom = findChild("mdia/minf/stbl/co64")) != nullptr) {
        auto co64 = DYNAMIC_CAST(Co64, atom);
        return co64->adjustChunkOffset(delta);
    }
    return ERROR_INVALID_STATE;
}

Result Trak::getChunkOffsets(Array<UI64> &chunkOffsets) {
    Atom* atom;
    if ((atom = findChild("mdia/minf/stbl/stco"))) {
        auto* stco = DYNAMIC_CAST(Stco, atom);
        if (!stco) {
            return ERROR_INTERNAL;
        }
        Cardinal stcoChunkCount = stco->getChunkCount();
        const UI32* stcoChunkOffsets = stco->getChunkOffsets();
        chunkOffsets.SetItemCount(stcoChunkCount);
        for (unsigned int i = 0; i < stcoChunkCount; i++) {
            chunkOffsets[i] = stcoChunkOffsets[i];
        }
        return SUCCESS;
    } else if ((atom = findChild("mdia/minf/stbo/co64"))) {
        auto* co64 = DYNAMIC_CAST(Co64, atom);
        if (!co64) {
            return ERROR_INTERNAL;
        }
        Cardinal co64ChunkCount = co64->getChunkCount();
        const UI64* co64ChunkOffsets = co64->getChunkOffsets();
        chunkOffsets.SetItemCount(co64ChunkCount);
        for (unsigned int i = 0; i < co64ChunkCount; i++) {
            chunkOffsets[i] = co64ChunkOffsets[i];
        }
        return SUCCESS;
    }
    return ERROR_INVALID_STATE;
}

Result Trak::setChunkOffsets(const Array<UI64> &chunkOffsets) {
    Atom* atom;
    if ((atom = findChild("mdia/minf/stbl/stco"))) {
        auto* stco = DYNAMIC_CAST(Stco, atom);
        if (!stco) {
            return ERROR_INTERNAL;
        }
        Cardinal stcoChunkCount = stco->getChunkCount();
        UI32* stcoChunkOffsets = stco->getChunkOffsets();
        if (stcoChunkCount > chunkOffsets.ItemCount()) {
            return ERROR_OUT_OF_RANGE;
        }
        for (unsigned int i = 0; i < stcoChunkCount; i++) {
            stcoChunkOffsets[i] = (UI32) chunkOffsets[i];
        }
        return SUCCESS;
    } else if ((atom = findChild("mdia/minf/stbo/co64"))) {
        auto* co64 = DYNAMIC_CAST(Co64, atom);
        if (!co64) {
            return ERROR_INTERNAL;
        }
        Cardinal co64ChunkCount = co64->getChunkCount();
        UI64* co64ChunkOffsets = co64->getChunkOffsets();
        if (co64ChunkCount > chunkOffsets.ItemCount()) {
            return ERROR_OUT_OF_RANGE;
        }
        for (unsigned int i = 0; i < co64ChunkCount; i++) {
            co64ChunkOffsets[i] = chunkOffsets[i];
        }
        return SUCCESS;
    }
    return ERROR_INVALID_STATE;
}

UI32 Trak::getId() {
    return tkhdAtom ? tkhdAtom->getTrackId() : 0;
}

Result Trak::setId(UI32 trackId) {
    if (tkhdAtom) {
        tkhdAtom->setTrackId(trackId);
        return SUCCESS;
    }
    return ERROR_INVALID_STATE;
}

UI64 Trak::getDuration() {
    return tkhdAtom ? tkhdAtom->getDuration() : 0;
}

Result Trak::setDuration(UI64 duration) {
    if (tkhdAtom) {
        tkhdAtom->setDuration(duration);
        return SUCCESS;
    }
    return ERROR_INVALID_STATE;
}

UI64 Trak::getMediaDuration() {
    return mdhdAtom ? mdhdAtom->getDuration() : 0;
}

Result Trak::setMediaDuration(UI32 duration) {
    if (mdhdAtom) {
        mdhdAtom->setDuration(duration);
        return SUCCESS;
    }
    return ERROR_INVALID_STATE;
}

UI32 Trak::getMediaTimeScale() {
    return mdhdAtom ? mdhdAtom->getTimeScale() : 0;
}

Result Trak::setMediaTimeScale(UI32 timeScale) {
    if (mdhdAtom) {
        mdhdAtom->setTimeScale(timeScale);
        return SUCCESS;
    }
    return ERROR_INVALID_STATE;
}

UI32 Trak::getWidth() {
    return tkhdAtom ? tkhdAtom->getWidth() : 0;
}

Result Trak::setWidth(UI32 width) {
    if (tkhdAtom) {
        tkhdAtom->setWidth(width);
        return SUCCESS;
    }
    return ERROR_INVALID_STATE;
}

UI32 Trak::getHeight() {
    return tkhdAtom ? tkhdAtom->getHeight() : 0;
}

Result Trak::setHeight(UI32 height) {
    if (tkhdAtom) {
        tkhdAtom->setHeight(height);
        return SUCCESS;
    }
    return ERROR_INVALID_STATE;
}

}
