//
// Created by wlanjie on 2018/3/13.
//

#ifndef MP4_AVCPARSER_H
#define MP4_AVCPARSER_H


#include "array.h"
#include "databuffer.h"
#include "nal_parser.h"

const unsigned int AVC_NAL_UNIT_TYPE_CODED_SLICE_OF_NON_IDR_PICTURE    = 1;
const unsigned int AVC_NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_A      = 2;
const unsigned int AVC_NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_B      = 3;
const unsigned int AVC_NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_C      = 4;
const unsigned int AVC_NAL_UNIT_TYPE_CODED_SLICE_OF_IDR_PICTURE        = 5;
const unsigned int AVC_NAL_UNIT_TYPE_SEI                               = 6;
const unsigned int AVC_NAL_UNIT_TYPE_SPS                               = 7;
const unsigned int AVC_NAL_UNIT_TYPE_PPS                               = 8;
const unsigned int AVC_NAL_UNIT_TYPE_ACCESS_UNIT_DELIMITER             = 9;

const unsigned int AVC_SLICE_TYPE_P                                    = 0;
const unsigned int AVC_SLICE_TYPE_B                                    = 1;
const unsigned int AVC_SLICE_TYPE_I                                    = 2;
const unsigned int AVC_SLICE_TYPE_SP                                   = 3;
const unsigned int AVC_SLICE_TYPE_SI                                   = 4;

const unsigned int AVC_SPS_MAX_ID                                      = 255;
const unsigned int AVC_SPS_MAX_NUM_REF_FRAMES_IN_PIC_ORDER_CNT_CYCLE   = 256;
const unsigned int AVC_SPS_MAX_SCALING_LIST_COUNT                      = 12;

const unsigned int AVC_PPS_MAX_ID                                      = 255;
const unsigned int AVC_PPS_MAX_SLICE_GROUPS                            = 256;
const unsigned int AVC_PPS_MAX_PIC_SIZE_IN_MAP_UNITS                   = 65536;

typedef struct {
    int scale[16];
} AvcScalingList4x4;

typedef struct {
    int scale[64];
} AvcScalingList8x8;

struct AvcSequenceParameterSet {
    AvcSequenceParameterSet();

    void getInfo(unsigned int& width, unsigned int& height);
    DataBuffer rawBytes;


    unsigned int profileIdc;
    unsigned int constraintSet0Flag;
    unsigned int constraintSet1Flag;
    unsigned int constraintSet2Flag;
    unsigned int constraintSet3Flag;
    unsigned int levelIdc;
    unsigned int seqParameterSetId;
    unsigned int chromaFormatIdc;
    unsigned int separateColourPlaneFlag;
    unsigned int bitDepthLumaMinus8;
    unsigned int bitDepthChromaMinus8;
    unsigned int qpprimeYZeroTransformBypassFlag;
    unsigned int seqScalingMatrixPresentFlag;
    AvcScalingList4x4 scalingList4x4[6];
    bool useDefaultScalingMatrix4x4[AVC_SPS_MAX_SCALING_LIST_COUNT];
    AvcScalingList8x8 scalingList8x8[6];
    unsigned char useDefaultScalingMatrix8x8[AVC_SPS_MAX_SCALING_LIST_COUNT];
    unsigned int log2MaxFrameNumMinus4;
    unsigned int picOrderCntType;
    unsigned int log2MaxPicOrderCntLsbMinus4;
    unsigned int deltaPicOrderAlwaysZeroFlags;
    int offsetForNonRefPic;
    int offsetForTopToBottomField;
    unsigned int numRefFramesInPicOrderCntCycle;
    unsigned int offsetForRefFrame[AVC_SPS_MAX_NUM_REF_FRAMES_IN_PIC_ORDER_CNT_CYCLE];
    unsigned int numRefFrames;
    unsigned int gapsInFrameNumValueAllowedFlag;
    unsigned int picWidthInMbsMinus1;
    unsigned int picHeightInMapUnitsMinus1;
    unsigned int frameMbsOnlyFlag;
    unsigned int mbAdaptiveFrameFieldFlag;
    unsigned int direct8x8InferenceFlag;
    unsigned int frameCroppingFlag;
    unsigned int frameCropLeftOffset;
    unsigned int frameCropRightOffset;
    unsigned int frameCropTopOffset;
    unsigned int frameCropBottomOffset;
};

struct AvcPictureParameterSet {
    AvcPictureParameterSet();

    DataBuffer rawBytes;

    unsigned int picParameterSetId;
    unsigned int seqParameterSetId;
    unsigned int entropyCodingModeFlag;
    unsigned int picOrderPresentFlag;
    unsigned int numSliceGroupsMinus1;
    unsigned int sliceGroupMapType;
    unsigned int runLengthMinus1[AVC_PPS_MAX_SLICE_GROUPS];
    unsigned int topLeft[AVC_PPS_MAX_SLICE_GROUPS];
    unsigned int bottomRight[AVC_PPS_MAX_SLICE_GROUPS];
    unsigned int sliceGroupChangeDirectionFlag;
    unsigned int sliceGroupChangeRateMinus1;
    unsigned int picSizeInMapUnitsMinus1;
    unsigned int numRefIdx10ActiveMinus1;
    unsigned int numRefIdx11ActiveMinus1;
    unsigned int weightedPredFlag;
    unsigned int weightedBipredIdc;
    int picInitQpMinus26;
    int picInitQsMinus26;
    int chromaQpIndexOffset;
    unsigned int deblockingFilterControlPresentFlag;
    unsigned int constrainedIntraPredFlag;
    unsigned int redundantPicCntPresentFlag;
};

struct AvcSliceHeader {
    AvcSliceHeader();

    unsigned int size;
    unsigned int firstMbInSlice;
    unsigned int sliceType;
    unsigned int picParameterSetId;
    unsigned int colourPlaneId;
    unsigned int frameNum;
    unsigned int fieldPicFlag;
    unsigned int bottomFieldFlag;
    unsigned int idrPicId;
    unsigned int picOrderCntLsb;
    int deltaPicOrderCnt[2];
    unsigned int redundantPicCnt;
    unsigned int directSpatialMvPredFlag;
    unsigned int numRefIdxActiveOverrideFlag;
    unsigned int numRefIdxL0ActiveMinus1;
    unsigned int numRefIdxL1ActiveMinus1;
    unsigned int refPicListReorderingFlagL0;
    unsigned int reorderingOfPicNumsIdc;
    unsigned int absDiffPicNumMinus1;
    unsigned int longTermPicNum;
    unsigned int refPicListReorderingFlagL1;
    unsigned int lumaLog2WeightDenom;
    unsigned int chromaLog2WeightDenom;
    unsigned int cabacInitIdc;
    unsigned int sliceQpDelta;
    unsigned int spForSwitchFlag;
    int sliceQsDelta;
    unsigned int disableDeblockingFilterIdc;
    int sliceAlphaC0OffsetDiv2;
    int sliceBetaOffsetDiv2;
    unsigned int sliceGroupChangeCycle;
    unsigned int noOutputOfPriorPicsFlag;
    unsigned int longTermReferenceFlag;
    unsigned int differenceOfPicNumsMinus1;
    unsigned int longTermFrameIdx;
    unsigned int maxLongTermFrameIdxPlus1;
};

class AvcNalParser : public NalParser {
public:
    static const char* naluTypeName(unsigned int naluType);
    static const char* primaryPicTypeName(unsigned int primaryPicType);
    static const char* sliceTypeName(unsigned int sliceType);
    AvcNalParser();

};

class AvcFrameParser {
public:
    struct AccessUnitInfo {
        Array<DataBuffer*> nalUnits;
        bool isIdr;
        UI32 decodeOrder;
        UI32 displayOrder;

        void reset();
    };

    AvcFrameParser();
    ~AvcFrameParser();
    /**
      * Feed some data to the parser and look for the next NAL Unit.
      *
      * @param data Pointer to the memory buffer with the data to feed.
      * @param data_size Size in bytes of the buffer pointed to by the
      * data pointer.
      * @param bytes_consumed Number of bytes from the data buffer that were
      * consumed and stored by the parser.
      * @param access_unit_info Reference to a AccessUnitInfo structure that will
      * contain information about any access unit found in the data. If no
      * access unit was found, the nal_units field of this structure will be an
      * empty array.
      * @param eos Boolean flag that indicates if this buffer is the last
      * buffer in the stream/file (End Of Stream).
      *
      * @result: SUCCESS is the call succeeds, or an error code if it
      * fails.
      *
      * The caller must not feed the same data twice. When this method
      * returns, the caller should inspect the value of bytes_consumed and
      * advance the input stream source accordingly, such that the next
      * buffer passed to this method will be exactly bytes_consumed bytes
      * after what was passed in this call. After all the input data has
      * been supplied to the parser (eos=true), the caller also should
      * this method with an empty buffer (data=NULL and/or data_size=0) until
      * no more data is returned, because there may be buffered data still
      * available.
      *
      * When data is returned in the access_unit_info structure, the caller is
      * responsible for freeing this data by calling AccessUnitInfo::Reset()
      */
    Result feed(const void* data,
                    Size dataSize,
                    Size& bytesConsumed,
                    AccessUnitInfo& accessUnitInfo,
                    bool eos = false);

    Result feed(const UI08* nalUnit,
                Size nalUnitSize,
                AccessUnitInfo& accessUnitInfo,
                bool lastUnit = false);

    AvcSequenceParameterSet** getSequenceParameterSet() { return &sps[0]; }
    AvcPictureParameterSet** getPictureParameterSet() { return &pps[0]; }

    Result parseSPS(const unsigned char* data, unsigned int dataSize, AvcSequenceParameterSet& sps);
    Result parsePPS(const unsigned char* data, unsigned int dataSize, AvcPictureParameterSet& pps);
    Result parseSliceHeader(const UI08* data,
                            unsigned int dataSize,
                            unsigned int nalUnitType,
                            unsigned int nalRefIdc,
                            AvcSliceHeader& sliceHeader);
private:
    bool sameFrame(unsigned int nalUnitType1,
                   unsigned int nalRefIdc1,
                   AvcSliceHeader& sh1,
                   unsigned int nalUnitType2,
                   unsigned int nalRefIdc2,
                   AvcSliceHeader& sh2);
    AvcSequenceParameterSet* getSliceSps(AvcSliceHeader& sh);
    void checkIfAccessUnitIsCompleted(AccessUnitInfo& accessUnitInfo);
    void appendNalUnitData(const unsigned char* data, unsigned int dataSize);

    AvcNalParser nalParser;
    AvcSequenceParameterSet* sps[AVC_SPS_MAX_ID + 1];
    AvcPictureParameterSet* pps[AVC_PPS_MAX_ID + 1];

    unsigned int nalUnitType;
    unsigned int nalRefIdc;
    AvcSliceHeader* sliceHeader;
    unsigned int accessUnitVclNalUnitCount;

    unsigned int totalNalUnitCount;
    unsigned int totalAccessUnitCount;
    Array<DataBuffer*> accessUnitData;

    unsigned int prevFrameNum;
    unsigned int prevFrameNumOffset;
    int prevPicOrderCntMsb;
    unsigned int prevPicOrderCntLsb;
};

#endif //MP4_AVCPARSER_H
