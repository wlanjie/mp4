//
// Created by wlanjie on 2018/3/10.
//

#ifndef MP4_SAMPLEENTRY_H
#define MP4_SAMPLEENTRY_H

#include "container.h"
#include "sampledescription.h"

namespace mp4 {

class SampleEntry : public Container {
public:
    SampleEntry(Atom::Type format, const AtomParent* details = nullptr);
    SampleEntry(Atom::Type format, Size size, ByteStream& stream, AtomFactory& factory);

    virtual ~SampleEntry() {}
    UI16 getDataReferenceIndex() { return dataReferenceIndex; }
    virtual Result write(ByteStream& stream);
    virtual SampleDescription* toSampleDescription();
    virtual Atom* clone();
    virtual void onChildChanged(Atom* child);
protected:
    SampleEntry(Atom::Type format, Size size);
    virtual void read(ByteStream& stream, AtomFactory& factory);
    virtual Size getFieldsSize();
    virtual Result readFields(ByteStream& stream);
    virtual Result writeFields(ByteStream& stream);
    UI08 reserved1[6];
    UI16 dataReferenceIndex;
};


class VisualSampleEntry : public SampleEntry {
public:
    VisualSampleEntry(Atom::Type format,
                      UI16 width,
                      UI16 height,
                      UI16 depth,
                      const char* compressorName,
                      const AtomParent* details = nullptr);

    VisualSampleEntry(Atom::Type format,
                      Size size,
                      ByteStream& stream,
                      AtomFactory& factory);

    UI16 getWidth() { return width; }
    UI16 getHeight() { return height; }
    UI32 getHorizResolution() { return horizResolution; }
    UI32 getVertResolution() { return vertResolution; }
    UI16 getDepth() { return depth; }
    const char* getCompressorName() { return compressorName.GetChars(); }

    SampleDescription* toSampleDescription();

protected:
    virtual Size getFieldsSize();
    virtual Result readFields(ByteStream& stream);
    virtual Result writeFields(ByteStream& stream);

    UI16   predefined1;     // = 0
    UI16   reserved2;       // = 0
    UI08   predefined2[12]; // = 0
    UI16   width;
    UI16   height;
    UI32   horizResolution; // = 0x00480000 (72 dpi)
    UI32   vertResolution;  // = 0x00480000 (72 dpi)
    UI32   reserved3;       // = 0
    UI16   frameCount;      // = 1
    String compressorName;
    UI16   depth;           // = 0x0018
    UI16   predefined3;     // = 0xFFFF
};

class AvcSampleEntry : public VisualSampleEntry {
public:
    AvcSampleEntry(UI32 format,
                   Size size,
                   ByteStream& stream,
                   AtomFactory& factory);
    AvcSampleEntry(UI32 format,
                   UI16 width,
                   UI16 height,
                   UI16 depth,
                   const char* compressorName,
                   const AtomParent* details);
    virtual SampleDescription* toSampleDescription();
};

class HevcSampleEntry : public VisualSampleEntry {
public:
    HevcSampleEntry(UI32 format,
                    Size size,
                    ByteStream& stream,
                    AtomFactory& factory);
    HevcSampleEntry(UI32 format,
                    UI16 width,
                    UI16 height,
                    UI16 depth,
                    const char* compressorName,
                    const AtomParent* details);
    virtual SampleDescription* toSampleDescription();
};

class AudioSampleEntry : public SampleEntry {
public:
    AudioSampleEntry(Atom::Type format, UI32 sampleRate, UI16 sampleSize, UI16 channelCount);
    AudioSampleEntry(Atom::Type format, Size size, ByteStream& stream, AtomFactory& factory);

    UI32 getSampleRete();
    UI16 getSampleSize() { return sampleSize; }
    UI16 getChannelCount();

    SampleDescription* toSampleDescription();

protected:
    virtual Size getFieldsSize();
    virtual Result readFields(ByteStream& stream);
    virtual Result writeFields(ByteStream& stream);

    UI16 qtVersion;
    UI16 qtRevision;
    UI32 qtVendor;
    UI16 channelCount;
    UI16 sampleSize;
    UI16 qtCompressionId;
    UI16 qtPacketSize;
    UI32 sampleRate;

    UI32 qtV1SamplesPerPacket;
    UI32 qtV1BytesPerPacket;
    UI32 qtV1BytesPerFrame;
    UI32 qtV1BytesPerSample;

    UI32 qtV2StructSize;
    double qtV2SampleRate64;
    UI32 qtV2ChannelCount;
    UI32 qtV2Reserved;
    UI32 qtV2BitsPerChannel;
    UI32 qtV2FormatSpecificFlags;
    UI32 qtV2BytesPerAudioPacket;
    UI32 qtV2LPCMFramesPerAudioPacket;
    DataBuffer qtV2Extension;
};

class MpegAudioSampleEntry : public AudioSampleEntry {
public:
    MpegAudioSampleEntry(UI32 type, UI32 sampleRate, UI16 sampleSize, UI16 channelCount, EsDescriptor* descriptor);
    MpegAudioSampleEntry(UI32 type, Size size, ByteStream& stream, AtomFactory& factory);
    SampleDescription* toSampleDescription();
};

class Mp4aSampleEntry : public MpegAudioSampleEntry {
public:
    Mp4aSampleEntry(Size size, ByteStream& stream, AtomFactory& factory);
    Mp4aSampleEntry(UI32 sampleRate, UI16 sampleSize, UI16 channelCount, EsDescriptor* descriptor);
};

}
#endif //MP4_SAMPLEENTRY_H
