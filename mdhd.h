//
// Created by wlanjie on 2018/3/9.
//

#ifndef MP4_MDHDATOM_H
#define MP4_MDHDATOM_H


#include "atom.h"

namespace mp4 {
/**
 * 定义了媒体的特性，例如time scale和duration。
 */
class Mdhd : public Atom {
public:
    static Mdhd* create(Size size, ByteStream& stream);

    Mdhd(UI32 creationTime,
             UI32 modificationTime,
             UI32 timeScale,
             UI64 duration,
             const char* language);
    virtual Result writeFields(ByteStream& stream);
    UI32 getDurationMs() const;
    UI64 getDuration() { return duration; }
    void setDuration(UI64 duration) { this->duration = duration; }
    UI32 getTimeScale() { return timeScale; }
    void setTimeScale(UI32 timeScale) { this->timeScale = timeScale; }
    const String& getLanguage() { return language; }
private:
    Mdhd(UI32 size,
             UI08 version,
             UI32 flags,
             ByteStream& stream);

    UI64 creationTime;
    UI64 modificationTime;
    UI32 timeScale;
    UI64 duration;
    String language;
};

}
#endif //MP4_MDHDATOM_H
