//
// Created by wlanjie on 2018/3/9.
//

#ifndef MP4_TKHDATOM_H
#define MP4_TKHDATOM_H

#include "atom.h"

const int TKHD_FLAG_TRACK_ENABLED    = 1;
const int TKHD_FLAG_TRACK_IN_MOVIE   = 2;
const int TKHD_FLAG_TRACK_IN_PREVIEW = 4;

const int TKHD_FLAG_DEFAULTS         = 7;

/**
 * tkhd (Track Header Atoms)
 * 每个trak都包含了一个track header atom. The track header atom 定义了一个track的特性，例如时间，空间和音量信息，它的类型是('tkhd')
 */
class Tkhd : public Atom {
public:
    static Tkhd* create(Size size, ByteStream& stream);
    /**
     *
     * @param creationTime 4字节 创建时间（相对于UTC时间1904-01-01零点的秒数）
     * @param modificationTime 4字节 修改时间
     * @param trackId 4字节 id号，不能重复且不能为0
     * @param duration 4字节 track的时间长度
     * @param volume 2字节 [8.8]格式，如果为音频track，1.0（0x0100）表示最大音量；否则为0 1.0为正常音量
     * @param width 4字节 宽
     * @param height 4字节 高
     * @param layer 2字节 视频层，默认为0，值小的在上层
     * @param alternateGroup 2字节 track分组信息，默认为0表示该track未与其他track有群组关系
     * @param matrix 36字节 视频变换矩阵
     */
    Tkhd(UI32 creationTime,
             UI32 modificationTime,
             UI32 trackId,
             UI64 duration,
             UI16 volume,
             UI32 width,
             UI32 height,
             UI16 layer = 0,
             UI16 alternateGroup = 0,
             const SI32* matrix = NULL);

    virtual Result writeFields(ByteStream& stream);

    void setCreationTime(UI64 creationTime) { this->creationTime = creationTime; }
    UI64 getCreationTime() const { return creationTime; }
    void setTrackId(UI32 trackId) { this->trackId = trackId; }
    UI32 getTrackId() const { return trackId; }
    void setDuration(UI64 duration) { this->duration = duration; }
    UI64 getDuration() const { return duration; }
    void setLayer(UI16 layer) { this->layer = layer; }
    UI16 getLayer() const { return layer; }
    void setAlternateGroup(UI16 alternateGroup) { this->alternateGroup = alternateGroup; }
    UI16 getAlternateGroup() const { return alternateGroup; }
    void setVolume(UI16 volume) { this->volume = volume; }
    UI16 getVolume() const { return volume; }
    void setWidth(UI32 width) { this->width = width; }
    UI32 getWidth() const { return width; }
    void setHeight(UI32 height) { this->height = height; }
    UI32 getHeight() const { return height; }
    const SI32* getMatrix() const { return matrix; }
private:
    Tkhd(UI32 size, UI08 version, UI32 flags, ByteStream& stream);
    UI64 creationTime;
    UI64 modificationTime;
    UI32 trackId;
    UI64 duration;
    UI16 volume;
    UI32 width;
    UI32 height;
    UI16 layer;
    UI16 alternateGroup;
    UI32 reserved1;
    UI32 reserved2[2];
    UI16 reserved3;
    SI32 matrix[9];
};


#endif //MP4_TKHDATOM_H
