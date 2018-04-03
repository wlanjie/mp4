LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := mp4

LOCAL_SRC_FILES := \
	  results.cc \
	  atomfactory.cc \
	  atom.cc \
	  bytestream.cc \
	  character.cc \
	  utils.cc \
	  databuffer.cc \
	  container.cc \
	  file.cc \
	  url.cc \
	  ftyp.cc \
	  moov.cc \
	  mvhd.cc \
	  trak.cc \
	  tkhd.cc \
	  track.cc \
	  mdhd.cc \
	  hdlr.cc \
	  vmhd.cc \
	  smhd.cc \
	  dref.cc \
	  sample.cc \
	  sampletable.cc \
	  sampledescription.cc \
	  sample_entry.cc \
	  synthetic_sampletable.cc \
	  stsd.cc \
	  debug.cc \
	  stts.cc \
	  stsc.cc \
	  stsz.cc \
	  stco.cc \
	  stss.cc \
	  co64.cc \
	  ctts.cc \
	  avc_sample_description.cc \
	  avcc.cc \
	  atom_sampletable.cc \
	  stz2.cc \
	  avc_parser.cc \
	  nal_parser.cc \
	  movie.cc \
	  esds.cc \
	  expandable.cc \
	  descriptor_factory.cc \
	  decoder_config_descriptor.cc \
	  decoder_specificinfo_descriptor.cc \
	  adts_parser.cc \
	  bit_stream.cc \
	  sl_config_descriptor.cc \
	  mp4_audio_info.cc \
	  es_descriptor.cc \
	  android_file_byte_stream.cc \

include $(BUILD_SHARED_LIBRARY)
