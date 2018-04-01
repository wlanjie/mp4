//
// Created by wlanjie on 2018/2/7.
//

#include <iostream>
#include "file.h"
#include "filebytestream.h"
#include "synthetic_sampletable.h"
#include "utils.h"
#include "avc_sample_description.h"
#include "avc_parser.h"
#include "debug.h"
#include "adts_parser.h"
#include "sample_source.h"

#define VIDEO_FRAME_RATE 24

using namespace mp4;

struct SampleOrder {
    SampleOrder(UI32 decodeOrder, UI32 displayOrder) : decodeOrder(decodeOrder), displayOrder(displayOrder) {}
    UI32 decodeOrder;
    UI32 displayOrder;
};

static void sortSamples(SampleOrder* array, unsigned int n) {
    if (n < 2) {
        return;
    }
    SampleOrder pivot = array[n / 2];
    SampleOrder* left  = array;
    SampleOrder* right = array + n - 1;
    while (left <= right) {
        if (left->displayOrder < pivot.displayOrder) {
            ++left;
            continue;
        }
        if (right->displayOrder > pivot.displayOrder) {
            --right;
            continue;
        }
        SampleOrder temp = *left;
        *left++ = *right;
        *right-- = temp;
    }
    sortSamples(array, (unsigned int)(right - array + 1));
    sortSamples(left, (unsigned int)(array + n - left));
}

class FileStorage {
public:
    static Result create(const char* baseName, FileStorage*& fileStorage);
    ~FileStorage() {
        stream->release();
        remove(fileName.getChars());
    }
    ByteStream* getStream() { return stream; }

private:
    FileStorage(const char* baseName) : stream(nullptr) {
        auto nameLength = StringLength(baseName);
        auto* fileName = new char[nameLength + 2];
        CopyMemory(fileName, baseName, nameLength);
        fileName[nameLength] = '-';
        fileName[nameLength + 1] = '\0';
        this->fileName = fileName;
        delete[] fileName;
    }
    ByteStream* stream;
    String fileName;
};

Result FileStorage::create(const char *baseName, FileStorage *&fileStorage) {
    fileStorage = nullptr;
    auto* object = new FileStorage(baseName);
    auto result = FileByteStream::create(object->fileName.getChars(), FileByteStream::STREAM_MODE_WRITE, object->stream);
    if (FAILED(result)) {
        return result;
    }
    fileStorage = object;
    return SUCCESS;
}

void addH264Track(FileStorage *storage, Movie* movie) {
    const char* inputName = "/Users/wlanjie/Desktop/mp4.h264";
    ByteStream* input;
    auto result = FileByteStream::create(inputName, FileByteStream::STREAM_MODE_READ, input);
    if (FAILED(result)) {
        return;
    }
    auto videoFrameRate = VIDEO_FRAME_RATE * 1000;
    Array<SampleOrder> sampleOrders;
    auto* sampleTable = new SyntheticSampleTable();
    AvcFrameParser parser;
    while (true) {
        bool eos;
        unsigned char inputBuffer[4096];
        Size bytesInBuffer = 0;
        result = input->readPartial(inputBuffer, sizeof(inputBuffer), bytesInBuffer);
        if (SUCCEEDED(result)) {
            eos = false;
        } else if (result == ERROR_EOS) {
            eos = true;
        } else {
            fprintf(stderr, "ERROR: failed to read from input file\n");
            break;
        }
        Size offset = 0;
        bool foundAccessUnit = false;
        do {
            AvcFrameParser::AccessUnitInfo accessUnitInfo;

            foundAccessUnit = false;
            Size bytesConsumed = 0;
            result = parser.feed(&inputBuffer[offset],
                                 bytesInBuffer,
                                 bytesConsumed,
                                 accessUnitInfo,
                                 eos);
            if (FAILED(result)) {
                fprintf(stderr, "ERROR: Feed() failed (%d)\n", result);
                break;
            }
            if (accessUnitInfo.nalUnits.ItemCount()) {
                // we got one access unit
                foundAccessUnit = true;
                // compute the total size of the sample data
                unsigned int sample_data_size = 0;
                for (unsigned int i=0; i<accessUnitInfo.nalUnits.ItemCount(); i++) {
                    sample_data_size += 4+ accessUnitInfo.nalUnits[i]->getDataSize();
                }

                // store the sample data
                Position position = 0;
                storage->getStream()->tell(position);
                for (unsigned int i=0; i<accessUnitInfo.nalUnits.ItemCount(); i++) {
                    storage->getStream()->writeUI32(accessUnitInfo.nalUnits[i]->getDataSize());
                    storage->getStream()->write(accessUnitInfo.nalUnits[i]->getData(),
                                                accessUnitInfo.nalUnits[i]->getDataSize());
                }

                // add the sample to the track
                sampleTable->addSample(*storage->getStream(), position, sample_data_size, 1000, 0, 0, 0, accessUnitInfo.isIdr);

                // remember the sample order
                sampleOrders.Append(SampleOrder(accessUnitInfo.decodeOrder, accessUnitInfo.displayOrder));

                // free the memory buffers
                accessUnitInfo.reset();
            }

            offset += bytesConsumed;
            bytesInBuffer -= bytesConsumed;
        } while (bytesInBuffer || foundAccessUnit);
        if (eos) {
            break;
        }
    }


    // adjust the sample CTS/DTS offsets based on the sample orders
    if (sampleOrders.ItemCount() > 1) {
        unsigned int start = 0;
        for (unsigned int i=1; i<=sampleOrders.ItemCount(); i++) {
            if (i == sampleOrders.ItemCount() || sampleOrders[i].displayOrder == 0) {
                // we got to the end of the GOP, sort it by display order
                sortSamples(&sampleOrders[start], i-start);
                start = i;
            }
        }
    }
    unsigned int max_delta = 0;
    for (unsigned int i=0; i<sampleOrders.ItemCount(); i++) {
        if (sampleOrders[i].decodeOrder > i) {
            unsigned int delta =sampleOrders[i].decodeOrder-i;
            if (delta > max_delta) {
                max_delta = delta;
            }
        }
    }
    for (unsigned int i=0; i<sampleOrders.ItemCount(); i++) {
        sampleTable->useSample(sampleOrders[i].decodeOrder).setCts(1000ULL*(UI64)(i+max_delta));
    }

    // check the video parameters
    AvcSequenceParameterSet* sps = NULL;
    for (unsigned int i=0; i<=AVC_SPS_MAX_ID; i++) {
        if (parser.getSequenceParameterSet()[i]) {
            sps = parser.getSequenceParameterSet()[i];
            break;
        }
    }
    if (sps == NULL) {
        fprintf(stderr, "ERROR: no sequence parameter set found in video\n");
        input->release();
        return;
    }
    unsigned int video_width = 0;
    unsigned int video_height = 0;
    sps->getInfo(video_width, video_height);
    debug("width = %d height = %d\n", video_width, video_height);

    // collect the SPS and PPS into arrays
    Array<DataBuffer> sps_array;
    for (unsigned int i=0; i<=AVC_SPS_MAX_ID; i++) {
        if (parser.getSequenceParameterSet()[i]) {
            sps_array.Append(parser.getSequenceParameterSet()[i]->rawBytes);
        }
    }
    Array<DataBuffer> pps_array;
    for (unsigned int i=0; i<=AVC_PPS_MAX_ID; i++) {
        if (parser.getPictureParameterSet()[i]) {
            pps_array.Append(parser.getPictureParameterSet()[i]->rawBytes);
        }
    }

    // setup the video the sample descripton
    AvcSampleDescription* sample_description =
            new AvcSampleDescription(SAMPLE_FORMAT_AVC1,
                                         video_width,
                                         video_height,
                                         24,
                                         "AVC Coding",
                                         sps->profileIdc,
                                         sps->levelIdc,
                                         sps->constraintSet0Flag<<7 |
                                         sps->constraintSet1Flag<<6 |
                                         sps->constraintSet2Flag<<5 |
                                         sps->constraintSet3Flag<<4,
                                         4,
                                         sps_array,
                                         pps_array);
    sampleTable->addSampleDescription(sample_description);

    UI32 movie_timescale      = 1000;
    UI32 media_timescale      = videoFrameRate;
    UI64 video_track_duration = convertTime(1000 * sampleTable->getSampleCount(), media_timescale, movie_timescale);
    UI64 video_media_duration = 1000*sampleTable->getSampleCount();

    // create a video track
    Track* track = new Track(Track::TYPE_VIDEO,
                                     sampleTable,
                                     0,                    // auto-select track id
                                     movie_timescale,      // movie time scale
                                     video_track_duration, // track duration
                                     videoFrameRate,     // media time scale
                                     video_media_duration, // media duration
                                     "und",                // language
                                     video_width<<16,      // width
                                     video_height<<16      // height
    );

    // cleanup
    input->release();
    movie->addTrack(track);
}

static void addAacTrack(FileStorage* storage, Movie* movie) {
    const char* inputName = "/Users/wlanjie/Desktop/test.aac";
    ByteStream* input;
    auto result = FileByteStream::create(inputName, FileByteStream::STREAM_MODE_READ, input);
    if (FAILED(result)) {
        return;
    }
    auto* sampleTable = new SyntheticSampleTable();
    // create an ADTS parser
    AdtsParser parser;
    bool initialized = false;
    unsigned int sampleDescriptionIndex = 0;

    // read from the input, feed, and get AAC frames
    UI32 sampleRate = 0;
    Cardinal sampleCount = 0;
    bool eos = false;
    for(;;) {
        // try to get a frame
        AacFrame frame;
        result = parser.findFrame(frame);
        if (SUCCEEDED(result)) {
            if (!initialized) {
                initialized = true;

                DataBuffer dsi;
                unsigned char aacDsi[2];

                unsigned int objectType = 2; // AAC LC by default
                aacDsi[0] = (objectType<<3) | (frame.info.samplingFrequencyIndex>>1);
                aacDsi[1] = ((frame.info.samplingFrequencyIndex&1)<<7) | (frame.info.channelConfiguration<<3);

                dsi.setData(aacDsi, 2);
                MpegAudioSampleDescription* sampleDescription =
                        new MpegAudioSampleDescription(
                                OTI_MPEG4_AUDIO,   // object type
                                (UI32)frame.info.samplingFrequency,
                                16,                    // sample size
                                frame.info.channelConfiguration,
                                &dsi,                  // decoder info
                                6144,                  // buffer size
                                128000,                // max bitrate
                                128000);               // average bitrate
                sampleDescriptionIndex = sampleTable->getSampleDescriptionCount();
                sampleTable->addSampleDescription(sampleDescription);
                sampleRate = (UI32)frame.info.samplingFrequency;
            }

            // read and store the sample data
            Position position = 0;
            storage->getStream()->tell(position);
            DataBuffer sample_data(frame.info.frameLength);
            sample_data.setDataSize(frame.info.frameLength);
            frame.source->readBytes(sample_data.useData(), frame.info.frameLength);
            storage->getStream()->write(sample_data.getData(), frame.info.frameLength);

            // add the sample to the table
            sampleTable->addSample(*storage->getStream(), position, frame.info.frameLength, 1024, sampleDescriptionIndex, 0, 0, true);
            sampleCount++;
        } else {
            if (eos) break;
        }

        // read some data and feed the parser
        UI08 input_buffer[4096];
        Size to_read = parser.getBytesFree();
        if (to_read) {
            Size bytes_read = 0;
            if (to_read > sizeof(input_buffer)) to_read = sizeof(input_buffer);
            result = input->readPartial(input_buffer, to_read, bytes_read);
            if (SUCCEEDED(result)) {
                Size to_feed = bytes_read;
                result = parser.feed(input_buffer, &to_feed);
                if (FAILED(result)) {
                    return;
                }
            } else {
                if (result == ERROR_EOS) {
                    eos = true;
                    parser.feed(NULL, NULL, BITSTREAM_FLAG_EOS);
                }
            }
        }
    }

    // create an audio track
    Track* track = new Track(Track::TYPE_AUDIO,
                                     sampleTable,
                                     0,                 // track id
                                     sampleRate,       // movie time scale
                                     sampleCount * 1024, // track duration
                                     sampleRate,       // media time scale
                                     sampleCount * 1024, // media duration
                                     "und",             // language
                                     0, 0);             // width, height

    input->release();
    movie->addTrack(track);
}

int writeVideo() {
    ByteStream *output = NULL;
    const char* outputName = "/Users/wlanjie/Desktop/mp4.mp4";
    Array<UI32> brands;
    brands.Append(FILE_BRAND_ISOM);
    brands.Append(FILE_BRAND_MP42);
    brands.Append(FILE_BRAND_AVC1);

    FileStorage* storage = nullptr;
    auto result = FileStorage::create(outputName, storage);
    if (FAILED(result)) {
        return -1;
    }
    result = FileByteStream::create(outputName, FileByteStream::STREAM_MODE_WRITE, output);
    if (FAILED(result)) {
        delete storage;
        return 1;
    }
    auto* movie = new Movie();
    File file(movie);
    file.setFileType(FILE_BRAND_MP42, 1, &brands[0], brands.ItemCount());
    addH264Track(storage, movie);
    addAacTrack(storage, movie);
    file.write(*output);
    output->release();
    delete storage;
    return 0;
}

static void writeSample(const DataBuffer& sample_data,
            DataBuffer&       prefix,
            unsigned int          nalu_length_size,
            ByteStream*       output)
{
    const unsigned char* data      = sample_data.getData();
    unsigned int         data_size = sample_data.getDataSize();

    // allocate a buffer for the PES packet
    DataBuffer frame_data;
    unsigned char* frame_buffer = NULL;

    // add a delimiter if we don't already have one
    bool have_access_unit_delimiter = (data_size >  nalu_length_size) && ((data[nalu_length_size] & 0x1F) == AVC_NAL_UNIT_TYPE_ACCESS_UNIT_DELIMITER);
    if (!have_access_unit_delimiter) {
        Size frame_data_size = frame_data.getDataSize();
        frame_data.setDataSize(frame_data_size + 6);
        frame_buffer = frame_data.useData()+frame_data_size;

        // start of access unit
        frame_buffer[0] = 0;
        frame_buffer[1] = 0;
        frame_buffer[2] = 0;
        frame_buffer[3] = 1;
        frame_buffer[4] = 9;    // NAL type = Access Unit Delimiter;
        frame_buffer[5] = 0xE0; // Slice types = ANY
    }

    // write the NAL units
    bool prefix_added = false;
    while (data_size) {
        // sanity check
        if (data_size < nalu_length_size) break;

        // get the next NAL unit
        UI32 nalu_size;
        if (nalu_length_size == 1) {
            nalu_size = *data++;
            data_size--;
        } else if (nalu_length_size == 2) {
            nalu_size = bytesToInt16BE(data);
            data      += 2;
            data_size -= 2;
        } else if (nalu_length_size == 4) {
            nalu_size = bytesToInt32BE(data);
            data      += 4;
            data_size -= 4;
        } else {
            break;
        }
        if (nalu_size > data_size) break;

        // add the prefix if needed
        if (prefix.getDataSize() && !prefix_added && !have_access_unit_delimiter) {
            Size frame_data_size = frame_data.getDataSize();
            frame_data.setDataSize(frame_data_size + prefix.getDataSize());
            frame_buffer = frame_data.useData()+frame_data_size;
            CopyMemory(frame_buffer, prefix.getData(), prefix.getDataSize());
            prefix_added = true;
        }

        // add a start code before the NAL unit
        Size frame_data_size = frame_data.getDataSize();
        frame_data.setDataSize(frame_data_size + 3 + nalu_size);
        frame_buffer = frame_data.useData()+frame_data_size;
        frame_buffer[0] = 0;
        frame_buffer[1] = 0;
        frame_buffer[2] = 1;
        CopyMemory(frame_buffer+3, data, nalu_size);

        // add the prefix if needed
        if (prefix.getDataSize() && !prefix_added) {
            Size frame_data_size = frame_data.getDataSize();
            frame_data.setDataSize(frame_data_size + prefix.getDataSize());
            frame_buffer = frame_data.useData()+frame_data_size;
            CopyMemory(frame_buffer, prefix.getData(), prefix.getDataSize());
            prefix_added = true;
        }

        // move to the next NAL unit
        data      += nalu_size;
        data_size -= nalu_size;
    }

    output->write(frame_data.getData(), frame_data.getDataSize());
}

///*----------------------------------------------------------------------
//|   MakeFramePrefix
//+---------------------------------------------------------------------*/
static Result
MakeFramePrefix(SampleDescription* sdesc, DataBuffer& prefix, unsigned int& nalu_length_size)
{
    AvcSampleDescription* avc_desc = DYNAMIC_CAST(AvcSampleDescription, sdesc);
    if (avc_desc == NULL) {
        fprintf(stderr, "ERROR: track does not contain an AVC stream\n");
        return FAILURE;
    }

    if (sdesc->getFormat() == SAMPLE_FORMAT_AVC3 ||
        sdesc->getFormat() == SAMPLE_FORMAT_AVC4 ||
        sdesc->getFormat() == SAMPLE_FORMAT_DVAV) {
        // no need for a prefix, SPS/PPS NALs should be in the elementary stream already
        return SUCCESS;
    }

    // make the SPS/PPS prefix
    nalu_length_size = avc_desc->getNaluLength();
    for (unsigned int i=0; i<avc_desc->getSequenceParameters().ItemCount(); i++) {
        DataBuffer& buffer = avc_desc->getSequenceParameters()[i];
        unsigned int prefix_size = prefix.getDataSize();
        prefix.setDataSize(prefix_size + 4 + buffer.getDataSize());
        unsigned char* p = prefix.useData()+prefix_size;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 1;
        CopyMemory(p, buffer.getData(), buffer.getDataSize());
    }
    for (unsigned int i=0; i<avc_desc->getPictureParameters().ItemCount(); i++) {
        DataBuffer& buffer = avc_desc->getPictureParameters()[i];
        unsigned int prefix_size = prefix.getDataSize();
        prefix.setDataSize(prefix_size + 4 + buffer.getDataSize());
        unsigned char* p = prefix.useData()+prefix_size;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 1;
        CopyMemory(p, buffer.getData(), buffer.getDataSize());
    }

    return SUCCESS;
}

static void writeSamples(Track* track, SampleDescription* sdesc, ByteStream* output) {
    // make the frame prefix
    unsigned int nalu_length_size = 0;
    DataBuffer prefix;
    if (FAILED(MakeFramePrefix(sdesc, prefix, nalu_length_size))) {
        return;
    }

    printf("duration = %d\n", track->getDurationMs());
    SampleSource *source = new TrackSampleSource(track);
    source->seekToTime(8000, true);
    Sample sample;
    DataBuffer data;
    Ordinal index = 0;
    while (SUCCEEDED(source->readNextSample(sample, data, index))) {
        auto time = convertTime(sample.getCts(), track->getMediaTimeScale(), 1000);
        printf("pts = %lld, dts = %lld current = %lld duration = %d \n", sample.getCts(), sample.getDts(), time, sample.getDuration());
        writeSample(data, prefix, nalu_length_size, output);
        index++;
    }
    delete source;
}


int readVideo() {
    const char* input_name = "/Users/wlanjie/Desktop/testfile.mp4";
    // create the input stream
    ByteStream* input = NULL;
    Result result = FileByteStream::create(input_name, FileByteStream::STREAM_MODE_READ, input);
    if (FAILED(result)) {
        fprintf(stderr, "ERROR: cannot open input (%d)\n", result);
    }

    const char* output_name = "/Users/wlanjie/Desktop/mp4.h264";
    // create the output stream
    ByteStream* output = NULL;
    result = FileByteStream::create(output_name, FileByteStream::STREAM_MODE_WRITE, output);
    if (FAILED(result)) {
        fprintf(stderr, "ERROR: cannot open output (%d)\n", result);
    }

    // open the file
    File* inputFile = new File(*input);
    auto* movie = inputFile->getMovie();
    auto* track = movie->getTrack(Track::TYPE_VIDEO);
    if (!track) {
        // release
        input->release();
        output->release();
        return -1;
    }
    auto* sampleDescription = track->getSampleDescription(0);

    switch(sampleDescription->getType()) {
        case SampleDescription::TYPE_AVC:
            Ordinal index;
            track->getSampleIndexForTimeStampMs(7000, index);
            printf("width = %d height = %d index = %d\n", track->getWidth()/65536, track->getHeight()/65536, index);
            writeSamples(track, sampleDescription, output);
            break;
    }

    if (!sampleDescription) {
        input->release();
        output->release();
        return -2;
    }
    return 0;
}

static unsigned int
getSamplingFrequencyIndex(unsigned int sampling_frequency)
{
    switch (sampling_frequency) {
        case 96000: return 0;
        case 88200: return 1;
        case 64000: return 2;
        case 48000: return 3;
        case 44100: return 4;
        case 32000: return 5;
        case 24000: return 6;
        case 22050: return 7;
        case 16000: return 8;
        case 12000: return 9;
        case 11025: return 10;
        case 8000:  return 11;
        case 7350:  return 12;
        default:    return 0;
    }
}


static Result
WriteAdtsHeader(ByteStream* output,
                unsigned int    frame_size,
                unsigned int    sampling_frequency_index,
                unsigned int    channel_configuration)
{
    unsigned char bits[7];

    bits[0] = 0xFF;
    bits[1] = 0xF1; // 0xF9 (MPEG2)
    bits[2] = 0x40 | (sampling_frequency_index << 2) | (channel_configuration >> 2);
    bits[3] = ((channel_configuration&0x3)<<6) | ((frame_size+7) >> 11);
    bits[4] = ((frame_size+7) >> 3)&0xFF;
    bits[5] = (((frame_size+7) << 5)&0xFF) | 0x1F;
    bits[6] = 0xFC;

    return output->write(bits, 7);

    /*
        0:  syncword 12 always: '111111111111'
        12: ID 1 0: MPEG-4, 1: MPEG-2
        13: layer 2 always: '00'
        15: protection_absent 1
        16: profile 2
        18: sampling_frequency_index 4
        22: private_bit 1
        23: channel_configuration 3
        26: original/copy 1
        27: home 1
        28: emphasis 2 only if ID == 0

        ADTS Variable header: these can change from frame to frame
        28: copyright_identification_bit 1
        29: copyright_identification_start 1
        30: aac_frame_length 13 length of the frame including header (in bytes)
        43: adts_buffer_fullness 11 0x7FF indicates VBR
        54: no_raw_data_blocks_in_frame 2
        ADTS Error check
        crc_check 16 only if protection_absent == 0
   */
}


static void
WriteSamples(Track*             track,
             SampleDescription* sdesc,
             ByteStream*        output)
{
    AudioSampleDescription* audio_desc = DYNAMIC_CAST(AudioSampleDescription, sdesc);
    if (audio_desc == NULL) {
        fprintf(stderr, "ERROR: sample description is not audio\n");
        return;
    }
    unsigned int sampling_frequency_index = getSamplingFrequencyIndex(audio_desc->getSampleRate());
    unsigned int channel_configuration    = audio_desc->getChannelCount();

    Sample     sample;
    DataBuffer data;
    Ordinal    index = 0;
    while (SUCCEEDED(track->readSample(index, sample, data))) {
        WriteAdtsHeader(output, sample.getSize(), sampling_frequency_index, channel_configuration);
        output->write(data.getData(), data.getDataSize());
        index++;
    }
}


int readAac() {

    ByteStream* input  = NULL;
    File* input_file   = NULL;
    ByteStream* output = NULL;
    Movie*      movie  = NULL;
    Track*      audio_track = NULL;

    const char* inputName = "/Users/wlanjie/Desktop/testfile.mp4";
    auto result = FileByteStream::create(inputName, FileByteStream::STREAM_MODE_READ, input);
    if (FAILED(result)) {
        fprintf(stderr, "ERROR: cannot open input (%d)\n", result);
//        goto end;
    }

    const char* outputName = "/Users/wlanjie/Desktop/mp4.aac";
    // create the output stream
    result = FileByteStream::create(outputName, FileByteStream::STREAM_MODE_WRITE, output);
    if (FAILED(result)) {
        fprintf(stderr, "ERROR: cannot open output (%d)\n", result);
        input->release();
        output->release();
        return -1;
    }

    // open the file
    input_file = new File(*input);

    // get the movie
    SampleDescription* sample_description;
    movie = input_file->getMovie();
    if (movie == NULL) {
        fprintf(stderr, "ERROR: no movie in file\n");
        input->release();
        output->release();
        return -1;
    }

    // get the audio track
    audio_track = movie->getTrack(Track::TYPE_AUDIO);
    if (audio_track == NULL) {
        fprintf(stderr, "ERROR: no audio track found\n");
        input->release();
        output->release();
        return -1;
    }

    // check that the track is of the right type
    sample_description = audio_track->getSampleDescription(0);
    if (sample_description == NULL) {
        fprintf(stderr, "ERROR: unable to parse sample description\n");
        input->release();
        output->release();
        return -1;
    }

    // show info
    printf("Audio Track:\n");
    printf("  duration: %d ms\n", audio_track->getDurationMs());
    printf("  sample count: %d\n", audio_track->getSampleCount());

    switch (sample_description->getType()) {
        case SampleDescription::TYPE_MPEG: {
            WriteSamples(audio_track, sample_description, output);
            break;
        }
    }

    return 0;
}

int main(int argc, char** argv) {
    readVideo();
    readAac();
    writeVideo();
    return 0;
}