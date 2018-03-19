//
// Created by wlanjie on 2018/3/9.
//

#ifndef MP4_TRAKATOM_H
#define MP4_TRAKATOM_H

#include "container.h"
#include "tkhd.h"
#include "mdhd.h"
#include "sampletable.h"

/**
 * moov中通常包含两个trak，一个视频索引，一个音频索引
 * “trak”也是一个container box，
 * 其子box包含了该track的媒体数据引用和描述（hint track除外）
 * 一个MP4文件中的媒体可以包含多个track
 * 且至少有一个track，这些track之间彼此独立
 * 有自己的时间和空间信息。“trak”必须包含一个“tkhd”和一个“mdia”
 * 此外还有很多可选的box（略）
 * 其中“tkhd”为track header box，“mdia”为media box
 * 该box是一个包含一些track媒体数据信息box的container box
 */
class Trak : public Container {
public:
    static Trak* create(Size size, ByteStream& stream, AtomFactory& factory) {
        return new Trak(size, stream, factory);
    }

    Trak(SampleTable* sampleTable,
         Atom::Type hdlrType,
         const char* hdlrName,
         UI32 trackId,
         UI32 creationTime,
         UI32 modificationTime,
         UI64 trackDuration,
         UI32 mediaTimeScale,
         UI64 mediaDuration,
         UI16 volume,
         const char* language,
         UI32 width,
         UI32 height,
         UI16 layer = 0,
         UI16 alternateGroup = 0,
         const SI32* matrix = NULL);

    Tkhd* getTkhd() const { return tkhdAtom; }
    Result adjustChunkOffsets(SI64 delta);
    Result getChunkOffsets(Array<UI64>& chunkOffsets);
    Result setChunkOffsets(const Array<UI64>& chunkOffsets);
    UI32 getId();
    Result setId(UI32 trackId);
    UI64 getDuration();
    Result setDuration(UI64 duration);
    UI64 getMediaDuration();
    Result setMediaDuration(UI32 duration);
    UI32 getMediaTimeScale();
    Result setMediaTimeScale(UI32 timeScale);
    UI32 getWidth();
    Result setWidth(UI32 width);
    UI32 getHeight();
    Result setHeight(UI32 height);
private:
    Trak(UI32 size, ByteStream& stream, AtomFactory& factory);
    Tkhd* tkhdAtom;
    Mdhd* mdhdAtom;
};


#endif //MP4_TRAKATOM_H
