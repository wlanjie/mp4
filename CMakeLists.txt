cmake_minimum_required(VERSION 3.7)
project(mp4)

add_definitions(-D CONFIG_HAVE_ASSERT_H)

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_BUILD_TYPE Release)

set(SOURCE_FILES
        mp4.cc
        results.cc
        atomfactory.cc
        atom.cc
        bytestream.cc
        string.cc
        utils.cc
        databuffer.cc
        container.cc
        file.cc
        url.cc
        ftyp.cc
        stdfilebytestream.cc
        filewriter.cc
        moov.cc
        mvhd.cc
        trak.cc
        tkhd.cc
        track.cc
        mdhd.cc
        hdlr.cc
        vmhd.cc
        smhd.cc
        dref.cc
        sample.cc
        sampletable.cc
        sampledescription.cc
        sample_entry.cc
        synthetic_sampletable.cc
        stsd.cc
        debug.cc
        stts.cc
        stsc.cc
        stsz.cc
        stco.cc
        stss.cc
        co64.cc
        ctts.cc
        avc_sample_description.cc
        avcc.cc
        atom_sampletable.cc
        stz2.cc
        avc_parser.cc
        nal_parser.cc
        movie.cc
        esds.cc
        expandable.cc
        descriptor_factory.cc
        decoder_config_descriptor.cc
        decoder_specificinfo_descriptor.cc
        adts_parser.cc
        bit_stream.cc
        sl_config_descriptor.cc
        mp4_audio_info.cc
        es_descriptor.cc)
add_library(mp4 ${SOURCE_FILES})

string(TOLOWER sample binary_name)
add_executable(${binary_name} main.cc)
target_link_libraries(${binary_name} mp4)