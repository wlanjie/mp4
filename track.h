//
// Created by wlanjie on 2018/3/9.
//

#ifndef MP4_TRACK_H
#define MP4_TRACK_H

#include "types.h"
#include "trak.h"
#include "moov.h"

namespace mp4 {

const UI32 TRACK_DEFAULT_MOVIE_TIMESCALE = 1000;
const UI32 TRACK_FLAG_ENABLED = 0x0001;
const UI32 TRACK_FLAG_IN_MOVIE = 0x0002;
const UI32 TRACK_FLAG_IN_PREVIEW = 0x0004;

class Track {
public:
    typedef enum {
        TYPE_UNKNOWN = 0,
        TYPE_AUDIO = 1,
        TYPE_VIDEO = 2
    } Type;

    Track(Type type,
          SampleTable* sampleTable,
          UI32 trackId,
          UI32 movieTimeScale,
          UI64 trackDuration,
          UI32 mediaTimeScale,
          UI64 mediaDuration,
          const char* language,
          UI32 width,
          UI32 height);

    Track(SampleTable* sampleTable,
          UI32 trackId,
          UI32 movieTimeScale,
          UI64 trackDuration,
          UI32 mediaTimeScale,
          UI64 mediaDuration,
          const Track* trackProtoType);
    Track(Trak& atom,
          ByteStream& stream,
          UI32 movieTimeScale);

    virtual ~Track();

    Track* clone(Result* result = nullptr);
    UI32 getFlags() const;
    void setFlags(UI32 flags);
    Track::Type getType() const { return type; }
    UI32 getHandlerType() const;
    UI64 getDuration() const;
    UI32 getDurationMs() const;
    UI32 getWidth() const;
    UI32 getHeight() const;
    Cardinal getSampleCount() const;
    Result getSample(Ordinal index, Sample& sample);
    Result readSample(Ordinal index, Sample& sample, DataBuffer& data);
    Result getSampleIndexForTimeStampMs(UI64 tsMs, Ordinal& index);
    Ordinal getNearestSyncSampleIndex(Ordinal index, bool before = true);
    SampleDescription* getSampleDescription(Ordinal index);
    Cardinal getSampleDescriptionCount();
    SampleTable* getSampleTable() { return sampleTable; }
    UI32 getId() const;
    void setId(UI32 trackId);
    const Trak* getTrakAtom() const { return trakAtom; }
    Trak* useTrakAtom() { return trakAtom; }
    void setMovieTimeScale(UI32 timeScale);
    UI32 getMovieTimeScale() const;
    UI32 getMediaTimeScale() const;
    UI64 getMediaDuration() const;
    const char* getTrackLanguage() const;
    Result attach(Moov* moovAtom);
private:
    Type type;
    Trak* trakAtom;
    UI32 movieTimeScale;
    bool trakAtomIsOwned;
    SampleTable* sampleTable;
    bool sampleTableIsOwned;
};

}
#endif //MP4_TRACK_H
