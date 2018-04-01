//
// Created by wlanjie on 2018/3/10.
//

#include <cstring>
#include "sample_entry.h"
#include "avc_sample_description.h"

namespace mp4 {

SampleEntry::SampleEntry(Atom::Type format, const AtomParent *details) :
        Container(format),
        dataReferenceIndex(1) {
    reserved1[0] = 0;
    reserved1[1] = 0;
    reserved1[2] = 0;
    reserved1[3] = 0;
    reserved1[4] = 0;
    reserved1[5] = 0;
    size32 += 8;
    if (details) {
        details->copyChildren(*this);
    }
}

SampleEntry::SampleEntry(Atom::Type format, Size size, ByteStream &stream, AtomFactory &factory) : Container(format, (UI64) size, false) {
    read(stream, factory);
}

SampleEntry::SampleEntry(Atom::Type format, Size size) : Container(format, (UI64) size, false) {
    reserved1[0] = 0;
    reserved1[1] = 0;
    reserved1[2] = 0;
    reserved1[3] = 0;
    reserved1[4] = 0;
    reserved1[5] = 0;
}

Result SampleEntry::write(ByteStream &stream) {
    Result result = writeHeader(stream);
    if (FAILED(result)) {
        return result;
    }
    result = writeFields(stream);
    if (FAILED(result)) {
        return result;
    }
    return children.apply(AtomListWriter(stream));
}

SampleDescription *SampleEntry::toSampleDescription() {
    return new SampleDescription(SampleDescription::TYPE_UNKNOWN, type, this);
}

Atom *SampleEntry::clone() {
    return Atom::clone();
}

void SampleEntry::onChildChanged(Atom *child) {
    UI64 size = getHeaderSize() + getFieldsSize();
    children.apply(AtomSizeAdder(size));
    size32 += (UI32) size;
    if (parent) {
        parent->onChildChanged(this);
    }
}

void SampleEntry::read(ByteStream &stream, AtomFactory &factory) {
    readFields(stream);
    Size payloadSize = (Size) (getSize() - getHeaderSize());
    Size fieldsSize = getFieldsSize();
    if (payloadSize > fieldsSize) {
        readChildren(factory, stream, payloadSize - fieldsSize);
    }
}

Size SampleEntry::getFieldsSize() {
    return 8;
}

Result SampleEntry::readFields(ByteStream &stream) {
    stream.read(reserved1, sizeof(reserved1));
    stream.readUI16(dataReferenceIndex);
    return SUCCESS;
}

Result SampleEntry::writeFields(ByteStream &stream) {
    Result result = stream.write(reserved1, sizeof(reserved1));
    if (FAILED(result)) {
        return result;
    }
    return stream.writeUI16(dataReferenceIndex);
}

VisualSampleEntry::VisualSampleEntry(Atom::Type format,
                                     UI16 width,
                                     UI16 height,
                                     UI16 depth,
                                     const char *compressorName,
                                     const AtomParent *details) :
        SampleEntry(format, details),
        predefined1(0),
        reserved2(0),
        width(width),
        height(height),
        horizResolution(0x00480000),
        vertResolution(0x00480000),
        reserved3(0),
        frameCount(1),
        compressorName(compressorName),
        depth(depth),
        predefined3(0xFFFF) {
    memset(predefined2, 0, sizeof(predefined2));
    size32 += 70;
}

VisualSampleEntry::VisualSampleEntry(Atom::Type format,
                                     Size size,
                                     ByteStream &stream,
                                     AtomFactory &factory)
        : SampleEntry(format, size) {
    read(stream, factory);
}

Size VisualSampleEntry::getFieldsSize() {
    return SampleEntry::getFieldsSize() + 70;
}

Result VisualSampleEntry::readFields(ByteStream &stream) {
    auto result = SampleEntry::readFields(stream);
    if (FAILED(result)) {
        return result;
    }
    stream.readUI16(predefined1);
    stream.readUI16(reserved2);
    stream.read(predefined2, sizeof(predefined2));
    stream.readUI16(width);
    stream.readUI16(height);
    stream.readUI32(horizResolution);
    stream.readUI32(vertResolution);
    stream.readUI32(reserved3);
    stream.readUI16(frameCount);

    UI08 compressor_name[33];
    compressor_name[32] = 0;
    stream.read(compressor_name, 32);
    UI08 name_length = compressor_name[0];
    if (name_length < 32) {
        compressor_name[name_length+1] = 0; // force null termination
        compressorName = (const char*)(&compressor_name[1]);
    }

    stream.readUI16(depth);
    stream.readUI16(predefined3);

    return SUCCESS;
}

Result VisualSampleEntry::writeFields(ByteStream &stream) {
    Result result;

    // write the fields of the base class
    result = SampleEntry::writeFields(stream);
    if (FAILED(result)) return result;

    // predefined1
    result = stream.writeUI16(predefined1);
    if (FAILED(result)) return result;

    // reserved2
    result = stream.writeUI16(reserved2);
    if (FAILED(result)) return result;

    // predefined2
    result = stream.write(predefined2, sizeof(predefined2));
    if (FAILED(result)) return result;

    // width
    result = stream.writeUI16(width);
    if (FAILED(result)) return result;

    // height
    result = stream.writeUI16(height);
    if (FAILED(result)) return result;

    // horizontal resolution
    result = stream.writeUI32(horizResolution);
    if (FAILED(result)) return result;

    // vertical resolution
    result = stream.writeUI32(vertResolution);
    if (FAILED(result)) return result;

    // reserved3
    result = stream.writeUI32(reserved3);
    if (FAILED(result)) return result;

    // frame count
    result = stream.writeUI16(frameCount);
    if (FAILED(result)) return result;

    // compressor name
    unsigned char compressor_name[32];
    unsigned int name_length = compressorName.getLength();
    if (name_length > 31) name_length = 31;
    compressor_name[0] = (unsigned char)name_length;
    for (unsigned int i=0; i<name_length; i++) {
        compressor_name[i+1] = compressorName[i];
    }
    for (unsigned int i=name_length+1; i<32; i++) {
        compressor_name[i] = 0;
    }
    result = stream.write(compressor_name, 32);
    if (FAILED(result)) return result;

    // depth
    result = stream.writeUI16(depth);
    if (FAILED(result)) return result;

    // predefined3
    result = stream.writeUI16(predefined3);
    if (FAILED(result)) return result;

    return result;
}

SampleDescription* VisualSampleEntry::toSampleDescription() {
    return new GenericVideoSampleDescription(
            type,
            width,
            height,
            depth,
            compressorName.getChars(),
            this);
}

AvcSampleEntry::AvcSampleEntry(UI32 format,
                               Size size,
                               ByteStream &stream,
                               AtomFactory &factory) :
        VisualSampleEntry(format, size, stream, factory){

}

AvcSampleEntry::AvcSampleEntry(UI32 format,
                               UI16 width,
                               UI16 height,
                               UI16 depth,
                               const char *compressorName,
                               const AtomParent *details) :
        VisualSampleEntry(format, width, height, depth, compressorName, details) {

}

SampleDescription *AvcSampleEntry::toSampleDescription() {
    return new AvcSampleDescription(type, width, height, depth, compressorName.getChars(), this);
}

HevcSampleEntry::HevcSampleEntry(UI32 format,
                                 Size size,
                                 ByteStream &stream,
                                 AtomFactory &factory) : VisualSampleEntry(format, size, stream, factory){

}

HevcSampleEntry::HevcSampleEntry(UI32 format, UI16 width, UI16 height, UI16 depth, const char *compressorName,
                                 const AtomParent *details) : VisualSampleEntry(format, width, height, depth,
                                                                                compressorName, details) {

}

SampleDescription *HevcSampleEntry::toSampleDescription() {
    // TODO hevc
//    return new HevcSampleDescription(type, width, height, depth, compressorName.getChars(), this);
    return nullptr;
}

AudioSampleEntry::AudioSampleEntry(Atom::Type format, UI32 sampleRate, UI16 sampleSize, UI16 channelCount) :
        SampleEntry(format),
        qtVersion(0),
        qtRevision(0),
        qtVendor(0),
        channelCount(channelCount),
        sampleSize(sampleSize),
        qtCompressionId(0),
        qtPacketSize(0),
        sampleRate(sampleRate),
        qtV1SamplesPerPacket(0),
        qtV1BytesPerPacket(0),
        qtV1BytesPerFrame(0),
        qtV1BytesPerSample(0),
        qtV2StructSize(0),
        qtV2SampleRate64(0),
        qtV2ChannelCount(0),
        qtV2Reserved(0),
        qtV2BitsPerChannel(0),
        qtV2FormatSpecificFlags(0),
        qtV2BytesPerAudioPacket(0),
        qtV2LPCMFramesPerAudioPacket(0) {
    size32 += 20;
}

AudioSampleEntry::AudioSampleEntry(Atom::Type format,
                                   Size size,
                                   ByteStream &stream,
                                   AtomFactory &factory)
        : SampleEntry(format, size) {
    read(stream, factory);
}

UI32 AudioSampleEntry::getSampleRete() {
    return qtVersion == 2 ? (UI32) qtV2SampleRate64 : sampleRate >> 16;
}

UI16 AudioSampleEntry::getChannelCount() {
    return qtVersion == 2 ? (UI16) qtV2ChannelCount : channelCount;
}

SampleDescription *AudioSampleEntry::toSampleDescription() {
    return new GenericAudioSampleDescription(type, getSampleRete(), getSampleSize(), getChannelCount(), this);
}

Size AudioSampleEntry::getFieldsSize() {
    auto size = SampleEntry::getFieldsSize() + 20;
    if (qtVersion == 1) {
        size += 16;
    } else {
        size += 36 + qtV2Extension.getDataSize();
    }
    return size;
}

Result AudioSampleEntry::readFields(ByteStream &stream) {
    auto result = SampleEntry::readFields(stream);
    if (FAILED(result)) {
        return result;
    }

    stream.readUI16(qtVersion);
    stream.readUI16(qtRevision);
    stream.readUI32(qtVendor);
    stream.readUI16(channelCount);
    stream.readUI16(sampleSize);
    stream.readUI16(qtCompressionId);
    stream.readUI16(qtPacketSize);
    stream.readUI32(sampleRate);

    if (qtVersion == 1) {
        stream.readUI32(qtV1SamplesPerPacket);
        stream.readUI32(qtV1BytesPerPacket);
        stream.readUI32(qtV1BytesPerFrame);
        stream.readUI32(qtV1BytesPerSample);
    } else if (qtVersion == 2) {
        stream.readUI32(qtV1SamplesPerPacket);
        stream.readDouble(qtV2SampleRate64);
        stream.readUI32(qtV2ChannelCount);
        stream.readUI32(qtV2Reserved);
        stream.readUI32(qtV2BitsPerChannel);
        stream.readUI32(qtV2FormatSpecificFlags);
        stream.readUI32(qtV2BytesPerAudioPacket);
        stream.readUI32(qtV2LPCMFramesPerAudioPacket);
        if (qtV2StructSize > 72) {
            unsigned int extSize = qtV2StructSize - 72;
            qtV2Extension.setDataSize(extSize);
            stream.read(qtV2Extension.useData(), extSize);
        }
        qtV1SamplesPerPacket = qtV1BytesPerPacket = qtV1BytesPerFrame = qtV1BytesPerSample = 0;
    } else {
        qtV1SamplesPerPacket = 0;
        qtV1BytesPerPacket = 0;
        qtV1BytesPerFrame = 0;
        qtV1BytesPerSample = 0;
        qtV2StructSize = 0;
        qtV2SampleRate64 = 0;
        qtV2ChannelCount = 0;
        qtV2Reserved = 0;
        qtV2BitsPerChannel = 0;
        qtV2FormatSpecificFlags = 0;
        qtV2BytesPerAudioPacket = 0;
        qtV2LPCMFramesPerAudioPacket = 0;
    }
    return SUCCESS;
}

Result AudioSampleEntry::writeFields(ByteStream &stream) {
    Result result = SampleEntry::writeFields(stream);
    if (FAILED(result)) {
        return result;
    }
    result = stream.writeUI16(qtVersion);
    if (FAILED(result)) {
        return result;
    }
    result = stream.writeUI16(qtRevision);
    if (FAILED(result)) {
        return result;
    }
    result = stream.writeUI32(qtVendor);
    if (FAILED(result)) {
        return result;
    }
    result = stream.writeUI16(channelCount);
    if (FAILED(result)) {
        return result;
    }
    result = stream.writeUI16(sampleSize);
    if (FAILED(result)) {
        return result;
    }
    result = stream.writeUI16(qtCompressionId);
    if (FAILED(result)) {
        return result;
    }
    result = stream.writeUI16(qtPacketSize);
    if (FAILED(result)) {
        return result;
    }
    result = stream.writeUI32(sampleRate);
    if (FAILED(result)) {
        return result;
    }
    if (qtVersion == 1) {
        result = stream.writeUI32(qtV1SamplesPerPacket);
        if (FAILED(result)) {
            return result;
        }
        result = stream.writeUI32(qtV1BytesPerPacket);
        if (FAILED(result)) {
            return result;
        }
        result = stream.writeUI32(qtV1BytesPerFrame);
        if (FAILED(result)) {
            return result;
        }
        result = stream.writeUI32(qtV1BytesPerSample);
        if (FAILED(result)) {
            return result;
        }
    } else if (qtVersion == 2) {
        stream.writeUI32(qtV2StructSize);
        stream.writeDouble(qtV2SampleRate64);
        stream.writeUI32(qtV2ChannelCount);
        stream.writeUI32(qtV2Reserved);
        stream.writeUI32(qtV2BitsPerChannel);
        stream.writeUI32(qtV2FormatSpecificFlags);
        stream.writeUI32(qtV2BytesPerAudioPacket);
        stream.writeUI32(qtV2LPCMFramesPerAudioPacket);
        if (qtV2Extension.getDataSize()) {
            stream.write(qtV2Extension.getData(), qtV2Extension.getDataSize());
        }
    }
    return SUCCESS;
}

MpegAudioSampleEntry::MpegAudioSampleEntry(UI32 type,
                                           UI32 sampleRate,
                                           UI16 sampleSize,
                                           UI16 channelCount,
                                           EsDescriptor *descriptor) :
        AudioSampleEntry(type, sampleRate, sampleSize, channelCount) {
    if (descriptor) {
        addChild(new Esds(descriptor));
    }
}

MpegAudioSampleEntry::MpegAudioSampleEntry(UI32 type,
                                           Size size,
                                           ByteStream &stream,
                                           AtomFactory &factory)
        : AudioSampleEntry(type, size, stream, factory) {

}

SampleDescription *MpegAudioSampleEntry::toSampleDescription() {
    auto* esds = DYNAMIC_CAST(Esds, getChild(ATOM_TYPE_ESDS));
    if (!esds) {
        if (qtVersion > 0) {
            esds = DYNAMIC_CAST(Esds, findChild("wave/esds"));
        }
    }
    return new MpegAudioSampleDescription(getSampleRete(), getSampleSize(), getChannelCount(), esds);
}

Mp4aSampleEntry::Mp4aSampleEntry(Size size, ByteStream &stream, AtomFactory &factory) :
        MpegAudioSampleEntry(ATOM_TYPE_MP4A, size, stream, factory) {

}

Mp4aSampleEntry::Mp4aSampleEntry(UI32 sampleRate, UI16 sampleSize, UI16 channelCount, EsDescriptor *descriptor) :
        MpegAudioSampleEntry(ATOM_TYPE_MP4A, sampleRate, sampleSize, channelCount, descriptor) {

}

}
