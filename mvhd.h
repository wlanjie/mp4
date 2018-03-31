//
// Created by wlanjie on 2018/3/8.
//

#ifndef MP4_MVHDATOM_H
#define MP4_MVHDATOM_H

#include "defined.h"
#include "atom.h"

namespace mp4 {

/**
 * mvhd box (movie header)
 * 一个文件中只能包含一个mvhd box
 * mvhd box是一个Full box
 * FullBox，是Box的扩展，Box结构的基础上在Header中增加8bits version和24bits flags
 * 包含了媒体的创建与修改时间时间刻度、默认音量、色域、时长等信息
 */
class Mvhd : public Atom {
public:
    static Mvhd* create(Size size, ByteStream& stream);
    /**
     *
     * duration / timescale = 可播放时长（s）
     *
     * @param creationTime 创建时间
     * @param modificationTime 修改时间
     * @param timeScale 文件的所有时间描述所采用的单位。0x3E8 = 1000，即将1s平均分为1000份，每份1ms
     * @param duration 可播放时长
     * @param rate 帧率
     * @param volume 音量
     */
    Mvhd(UI32 creationTime, UI32 modificationTime, UI32 timeScale, UI64 duration, UI32 rate, UI16 volume);
    virtual Result writeFields(ByteStream& stream);
    UI64 getDuration() { return duration; }
    void setDuration(UI64 duration) { this->duration = duration; }
    UI32 getDurationMs();
    UI32 getTimeScale() { return timeScale; }
    void setTimeScale(UI32 timeScale) { this->timeScale = timeScale; }

private:
    Mvhd(UI32 size, UI08 version, UI32 flags, ByteStream& stream);

    UI64 creationTime;
    UI64 modificationTime;
    UI32 timeScale;
    UI64 duration;
    UI32 rate;
    UI16 volume;
    UI08 reserved1[2];
    UI08 reserved2[8];
    UI32 matrix[9];
    UI08 predefined[24];
    UI32 nextTrackId;
};

}
#endif //MP4_MVHDATOM_H
