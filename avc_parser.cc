//
// Created by wlanjie on 2018/3/13.
//

#include "avc_parser.h"
#include "utils.h"
#include "interfaces.h"
#include "debug.h"

AvcSequenceParameterSet::AvcSequenceParameterSet() :
        profileIdc(0),
        constraintSet0Flag(0),
        constraintSet1Flag(0),
        constraintSet2Flag(0),
        constraintSet3Flag(0),
        levelIdc(0),
        seqParameterSetId(0),
        chromaFormatIdc(0),
        separateColourPlaneFlag(0),
        bitDepthLumaMinus8(0),
        bitDepthChromaMinus8(0),
        qpprimeYZeroTransformBypassFlag(0),
        seqScalingMatrixPresentFlag(0),
        log2MaxFrameNumMinus4(0),
        picOrderCntType(0),
        log2MaxPicOrderCntLsbMinus4(0),
        deltaPicOrderAlwaysZeroFlags(0),
        offsetForNonRefPic(0),
        offsetForTopToBottomField(0),
        numRefFramesInPicOrderCntCycle(0),
        numRefFrames(0),
        gapsInFrameNumValueAllowedFlag(0),
        picWidthInMbsMinus1(0),
        picHeightInMapUnitsMinus1(0),
        frameMbsOnlyFlag(0),
        mbAdaptiveFrameFieldFlag(0),
        direct8x8InferenceFlag(0),
        frameCroppingFlag(0),
        frameCropLeftOffset(0),
        frameCropRightOffset(0),
        frameCropTopOffset(0),
        frameCropBottomOffset(0) {
    SetMemory(scalingList4x4, 0, sizeof(scalingList4x4));
    SetMemory(useDefaultScalingMatrix4x4, 0, sizeof(useDefaultScalingMatrix4x4));
    SetMemory(scalingList8x8, 0, sizeof(scalingList8x8));
    SetMemory(useDefaultScalingMatrix8x8, 0, sizeof(useDefaultScalingMatrix8x8));
    SetMemory(offsetForRefFrame, 0, sizeof(offsetForRefFrame));
}

void AvcSequenceParameterSet::getInfo(unsigned int &width, unsigned int &height) {
    width = (picWidthInMbsMinus1 + 1) * 16;
    height = (2 - frameMbsOnlyFlag) * (picHeightInMapUnitsMinus1 + 1) * 16;
    if (frameCroppingFlag) {
        auto cropH = 2 * (frameCropLeftOffset + frameCropRightOffset);
        auto cropV = 2 * (frameCropTopOffset + frameCropBottomOffset) * (2 - frameMbsOnlyFlag);
        if (cropH < width) {
            width -= cropH;
        }
        if (cropV < height) {
            height -= cropV;
        }
    }
}

AvcPictureParameterSet::AvcPictureParameterSet() :
        picParameterSetId(0),
        seqParameterSetId(0),
        entropyCodingModeFlag(0),
        picOrderPresentFlag(0),
        numSliceGroupsMinus1(0),
        sliceGroupMapType(0),
        sliceGroupChangeDirectionFlag(0),
        sliceGroupChangeRateMinus1(0),
        picSizeInMapUnitsMinus1(0),
        numRefIdx10ActiveMinus1(0),
        numRefIdx11ActiveMinus1(0),
        weightedPredFlag(0),
        weightedBipredIdc(0),
        picInitQpMinus26(0),
        picInitQsMinus26(0),
        chromaQpIndexOffset(0),
        deblockingFilterControlPresentFlag(0),
        constrainedIntraPredFlag(0),
        redundantPicCntPresentFlag(0) {
    SetMemory(runLengthMinus1, 0, sizeof(runLengthMinus1));
    SetMemory(topLeft, 0, sizeof(topLeft));
    SetMemory(bottomRight, 0, sizeof(bottomRight));
}

AvcSliceHeader::AvcSliceHeader() :
        size(0),
        firstMbInSlice(0),
        sliceType(0),
        picParameterSetId(0),
        colourPlaneId(0),
        frameNum(0),
        fieldPicFlag(0),
        bottomFieldFlag(0),
        idrPicId(0),
        picOrderCntLsb(0),
        redundantPicCnt(0),
        directSpatialMvPredFlag(0),
        numRefIdxActiveOverrideFlag(0),
        numRefIdxL0ActiveMinus1(0),
        numRefIdxL1ActiveMinus1(0),
        refPicListReorderingFlagL0(0),
        reorderingOfPicNumsIdc(0),
        absDiffPicNumMinus1(0),
        longTermPicNum(0),
        refPicListReorderingFlagL1(0),
        lumaLog2WeightDenom(0),
        chromaLog2WeightDenom(0),
        cabacInitIdc(0),
        sliceQpDelta(0),
        spForSwitchFlag(0),
        sliceQsDelta(0),
        disableDeblockingFilterIdc(0),
        sliceAlphaC0OffsetDiv2(0),
        sliceBetaOffsetDiv2(0),
        sliceGroupChangeCycle(0),
        noOutputOfPriorPicsFlag(0),
        longTermReferenceFlag(0),
        differenceOfPicNumsMinus1(0),
        longTermFrameIdx(0),
        maxLongTermFrameIdxPlus1(0) {
    deltaPicOrderCnt[0] = deltaPicOrderCnt[1] = 0;
}

const char *AvcNalParser::naluTypeName(unsigned int naluType) {
    switch (naluType) {
        case  0: return "Unspecified";
        case  1: return "Coded slice of a non-IDR picture";
        case  2: return "Coded slice data partition A";
        case  3: return "Coded slice data partition B";
        case  4: return "Coded slice data partition C";
        case  5: return "Coded slice of an IDR picture";
        case  6: return "Supplemental enhancement information (SEI)";
        case  7: return "Sequence parameter set";
        case  8: return "Picture parameter set";
        case  9: return "Access unit delimiter";
        case 10: return "End of sequence";
        case 11: return "End of stream";
        case 12: return "Filler data";
        case 13: return "Sequence parameter set extension";
        case 14: return "Prefix NAL unit in scalable extension";
        case 15: return "Subset sequence parameter set";
        case 19: return "Coded slice of an auxiliary coded picture without partitioning";
        case 20: return "Coded slice in scalable extension";
        default: return nullptr;
    }
}

const char *AvcNalParser::primaryPicTypeName(unsigned int primaryPicType) {
    switch (primaryPicType) {
        case 0: return "I";
        case 1: return "I, P";
        case 2: return "I, P, B";
        case 3: return "SI";
        case 4: return "SI, SP";
        case 5: return "I, SI";
        case 6: return "I, SI, P, SP";
        case 7: return "I, SI, P, SP, B";
        default: return nullptr;
    }
}

const char *AvcNalParser::sliceTypeName(unsigned int sliceType) {
    switch (sliceType) {
        case 0: return "P";
        case 1: return "B";
        case 2: return "I";
        case 3:	return "SP";
        case 4: return "SI";
        case 5: return "P";
        case 6: return "B";
        case 7: return "I";
        case 8:	return "SP";
        case 9: return "SI";
        default: return nullptr;
    }
}

AvcNalParser::AvcNalParser() : NalParser() {

}

AvcFrameParser::AvcFrameParser() :
        nalUnitType(0),
        nalRefIdc(0),
        sliceHeader(nullptr),
        accessUnitVclNalUnitCount(0),
        totalNalUnitCount(0),
        totalAccessUnitCount(0),
        prevFrameNum(0),
        prevFrameNumOffset(0),
        prevPicOrderCntMsb(0),
        prevPicOrderCntLsb(0) {
    for (unsigned i = 0; i < 256; i++) {
        pps[i] = nullptr;
        sps[i] = nullptr;
    }
}

AvcFrameParser::~AvcFrameParser() {
    for (unsigned int i = 0; i < 256; i++) {
        delete pps[i];
        delete sps[i];
    }
    delete sliceHeader;
    for (unsigned int i = 0; i < accessUnitData.ItemCount(); i++) {
        delete accessUnitData[i];
    }
}

Result AvcFrameParser::feed(const void *data, Size dataSize, Size &bytesConsumed,
                            AvcFrameParser::AccessUnitInfo &accessUnitInfo, bool eos) {
    const DataBuffer* nalUnit = nullptr;
    auto result = nalParser.feed(data, dataSize, bytesConsumed, nalUnit, eos);
    if (FAILED(result)) {
        return result;
    }
    if (bytesConsumed < dataSize) {
        eos = false;
    }
    return feed(nalUnit ? nalUnit->getData() : nullptr, nalUnit ? nalUnit->getDataSize() : 0, accessUnitInfo, eos);
}

Result AvcFrameParser::feed(const UI08 *nalUnit, Size nalUnitSize, AvcFrameParser::AccessUnitInfo &accessUnitInfo,
                            bool lastUnit) {
    Result result;
    accessUnitInfo.reset();
    if (nalUnit && nalUnitSize) {
        unsigned int nalUnitType = nalUnit[0] & 0x1F;
        const char* nalUnitTypeName = AvcNalParser::naluTypeName(nalUnitType);
        unsigned int nalRefIdc = (nalUnit[0] >> 5) & 3;
        if (!nalUnitTypeName) {
            nalUnitTypeName = "UNKNOWN";
        }
//        debug("Nalu %5d: ref=%d, size=%5d, type=%02d (%s)\n", totalNalUnitCount, nalRefIdc, nalUnitSize, nalUnitType, nalUnitTypeName);
        if (nalUnitType == AVC_NAL_UNIT_TYPE_ACCESS_UNIT_DELIMITER) {
            unsigned int primaryPicType = (nalUnit[1] >> 5);
            const char* primaryPicTypeName = AvcNalParser::primaryPicTypeName(primaryPicType);
            if (!primaryPicTypeName) {
                primaryPicTypeName = "UNKNOWN";
            }
//            debug("[%d:%s]\n", primaryPicType, primaryPicTypeName);
            checkIfAccessUnitIsCompleted(accessUnitInfo);
        } else if (nalUnitType == AVC_NAL_UNIT_TYPE_CODED_SLICE_OF_NON_IDR_PICTURE ||
                nalUnitType == AVC_NAL_UNIT_TYPE_CODED_SLICE_OF_IDR_PICTURE ||
                nalUnitType == AVC_NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_A) {
            auto* sliceHeader = new AvcSliceHeader;
            result = parseSliceHeader(nalUnit + 1, nalUnitSize - 1, nalUnitType, nalRefIdc, *sliceHeader);
            if (FAILED(result)) {
                return ERROR_INVALID_FORMAT;
            }
            const char* sliceTypeName = AvcNalParser::sliceTypeName(sliceHeader->sliceType);
            if (!sliceTypeName) {
                sliceTypeName = "?";
            }
//            debug(" pps_id=%d, header_size=%d, frame_num=%d, slice_type=%d (%s), \n ",
//                  sliceHeader->picParameterSetId,
//                  sliceHeader->size,
//                  sliceHeader->frameNum,
//                  sliceHeader->sliceType,
//                         sliceTypeName);
//            debug("\n");
            if (sliceHeader) {
                if (this->sliceHeader && !sameFrame(this->nalUnitType, this->nalRefIdc, *this->sliceHeader, nalUnitType, nalRefIdc, *sliceHeader)) {
                    checkIfAccessUnitIsCompleted(accessUnitInfo);
                    this->accessUnitVclNalUnitCount = 1;
                } else {
                    ++this->accessUnitVclNalUnitCount;
                }
            }
            appendNalUnitData(nalUnit, nalUnitSize);
            delete this->sliceHeader;
            this->sliceHeader = sliceHeader;
            this->nalUnitType = nalUnitType;
            this->nalRefIdc = nalRefIdc;
        } else if (nalUnitType == AVC_NAL_UNIT_TYPE_PPS) {
            auto* pps = new AvcPictureParameterSet;
            result = parsePPS(nalUnit, nalUnitSize, *pps);
            if (FAILED(result)) {
                delete pps;
            } else {
                delete this->pps[pps->picParameterSetId];
                this->pps[pps->picParameterSetId] = pps;
//                debug("PPS spsId = %d, ppsId=%d\n", pps->seqParameterSetId, pps->picParameterSetId);
                appendNalUnitData(nalUnit, nalUnitSize);
                checkIfAccessUnitIsCompleted(accessUnitInfo);
            }
        } else if (nalUnitType == AVC_NAL_UNIT_TYPE_SPS) {
            auto* sps = new AvcSequenceParameterSet;
            result = parseSPS(nalUnit, nalUnitSize, *sps);
            if (FAILED(result)) {
                delete sps;
            } else {
                delete this->sps[sps->seqParameterSetId];
                this->sps[sps->seqParameterSetId] = sps;
//                debug("SPS spsId = %d\n", sps->seqParameterSetId);
                checkIfAccessUnitIsCompleted(accessUnitInfo);
            }
        } else if (nalUnitType == AVC_NAL_UNIT_TYPE_SEI) {
            appendNalUnitData(nalUnit, nalUnitSize);
            checkIfAccessUnitIsCompleted(accessUnitInfo);
//            debug("\n");
        } else if (nalUnitType >= 14 && nalUnitType <= 18) {
            checkIfAccessUnitIsCompleted(accessUnitInfo);
        }
        this->totalNalUnitCount++;
    }
    if (lastUnit && accessUnitInfo.nalUnits.ItemCount() == 0) {
        checkIfAccessUnitIsCompleted(accessUnitInfo);
    }
    return SUCCESS;
}

static unsigned int readGolomb(BitReader& bits) {
    unsigned int leadingZeros = 0;
    while (bits.ReadBit() == 0) {
        leadingZeros++;
        if (leadingZeros > 32) {
            return 0;
        }
    }
    if (leadingZeros) {
        return (1 << leadingZeros) - 1 + bits.ReadBits(leadingZeros);
    }
    return 0;
}

static int signedGolomb(unsigned int codeNum) {
    if (codeNum % 2) {
        return (codeNum+1)/2;
    } else {
        return -((int)codeNum/2);
    }
}

Result AvcFrameParser::parseSPS(const unsigned char *data, unsigned int dataSize, AvcSequenceParameterSet &sps) {
    sps.rawBytes.setData(data, dataSize);
    DataBuffer unescaped(data, dataSize);
    NalParser::unescape(unescaped);
    BitReader bits(unescaped.getData(), unescaped.getDataSize());
    bits.SkipBits(8); // NAL Unit Type

    sps.profileIdc = bits.ReadBits(8);
    sps.constraintSet0Flag = bits.ReadBit();
    sps.constraintSet1Flag = bits.ReadBit();
    sps.constraintSet2Flag = bits.ReadBit();
    sps.constraintSet3Flag = bits.ReadBit();
    bits.SkipBits(4);
    sps.levelIdc = bits.ReadBits(8);
    sps.seqParameterSetId = readGolomb(bits);
    if (sps.seqParameterSetId > AVC_SPS_MAX_ID) {
        return ERROR_INVALID_FORMAT;
    }
    if (sps.profileIdc == 100 ||
            sps.profileIdc == 110 ||
            sps.profileIdc == 122 ||
            sps.profileIdc == 244 ||
            sps.profileIdc == 44 ||
            sps.profileIdc == 83 ||
            sps.profileIdc == 86) {
        sps.chromaFormatIdc = readGolomb(bits);
        sps.separateColourPlaneFlag = 0;
        if (sps.chromaFormatIdc == 3) {
            sps.separateColourPlaneFlag = bits.ReadBit();
        }
        sps.bitDepthLumaMinus8 = readGolomb(bits);
        sps.bitDepthChromaMinus8 = readGolomb(bits);
        sps.qpprimeYZeroTransformBypassFlag = bits.ReadBit();
        sps.seqScalingMatrixPresentFlag = bits.ReadBit();
        if (sps.seqScalingMatrixPresentFlag) {
            for (int i = 0; i < (sps.chromaFormatIdc != 3 ? 8 : 12); i++) {
                unsigned int seqScalingListPresentFlag = bits.ReadBit();
                if (seqScalingListPresentFlag) {
                    if (i < 6) {
                        int lastScale = 8;
                        int nextScale = 8;
                        for (unsigned int j = 0; j < 16; j++) {
                            if (nextScale) {
                                int deltaScale = signedGolomb(readGolomb(bits));
                                nextScale = (lastScale + deltaScale + 256) % 256;
                                sps.useDefaultScalingMatrix4x4[i] = (j == 0 && nextScale == 0);
                            }
                            sps.scalingList4x4[i].scale[j] = (nextScale == 0 ? lastScale : nextScale);
                            lastScale = sps.scalingList4x4[i].scale[j];
                        }
                    } else {
                        int lastScale = 8;
                        int nextScale = 8;
                        for (unsigned int j = 0; j < 64; j++) {
                            if (nextScale) {
                                int deltaScale = signedGolomb(readGolomb(bits));
                                nextScale = (lastScale + deltaScale + 256) % 256;
                                sps.useDefaultScalingMatrix8x8[i - 6] = (j == 0 && nextScale == 0);
                            }
                            sps.scalingList8x8[i - 6].scale[j] = (nextScale == 0 ? lastScale : nextScale);
                            lastScale = sps.scalingList8x8[i - 6].scale[j];
                        }
                    }
                }
            }
        }
    }

    sps.log2MaxFrameNumMinus4 = readGolomb(bits);
    sps.picOrderCntType = readGolomb(bits);
    if (sps.picOrderCntType > 2) {
        return ERROR_INVALID_FORMAT;
    }
    if (sps.picOrderCntType == 0) {
        sps.log2MaxPicOrderCntLsbMinus4 = readGolomb(bits);
    } else if (sps.picOrderCntType == 1) {
        sps.deltaPicOrderAlwaysZeroFlags = bits.ReadBit();
        sps.offsetForNonRefPic = signedGolomb(readGolomb(bits));
        sps.offsetForTopToBottomField = signedGolomb(readGolomb(bits));
        sps.numRefFramesInPicOrderCntCycle = readGolomb(bits);
        if (sps.numRefFramesInPicOrderCntCycle > AVC_SPS_MAX_NUM_REF_FRAMES_IN_PIC_ORDER_CNT_CYCLE) {
            return ERROR_INVALID_FORMAT;
        }
        for (unsigned int i = 0; i < sps.numRefFramesInPicOrderCntCycle; i++) {
            sps.offsetForRefFrame[i] = signedGolomb(readGolomb(bits));
        }
    }
    sps.numRefFrames = readGolomb(bits);
    sps.gapsInFrameNumValueAllowedFlag = bits.ReadBit();
    sps.picWidthInMbsMinus1 = readGolomb(bits);
    sps.picHeightInMapUnitsMinus1 = readGolomb(bits);
    sps.frameMbsOnlyFlag = bits.ReadBit();
    if (!sps.frameMbsOnlyFlag) {
        sps.mbAdaptiveFrameFieldFlag = bits.ReadBit();
    }
    sps.direct8x8InferenceFlag = bits.ReadBit();
    sps.frameCroppingFlag = bits.ReadBit();
    if (sps.frameCroppingFlag) {
        sps.frameCropLeftOffset = readGolomb(bits);
        sps.frameCropRightOffset = readGolomb(bits);
        sps.frameCropTopOffset = readGolomb(bits);
        sps.frameCropBottomOffset = readGolomb(bits);
    }
    return 0;
}

Result AvcFrameParser::parsePPS(const unsigned char *data, unsigned int dataSize, AvcPictureParameterSet &pps) {
    pps.rawBytes.setData(data, dataSize);
    DataBuffer unescaped(data, dataSize);
    NalParser::unescape(unescaped);
    BitReader bits(unescaped.getData(), unescaped.getDataSize());
    bits.SkipBits(8); // NAL Unit Type

    pps.picParameterSetId = readGolomb(bits);
    if (pps.picParameterSetId > AVC_PPS_MAX_ID) {
        return ERROR_INVALID_FORMAT;
    }
    pps.entropyCodingModeFlag = bits.ReadBit();
    pps.picOrderPresentFlag = bits.ReadBit();
    pps.numSliceGroupsMinus1 = readGolomb(bits);
    if (pps.numSliceGroupsMinus1 >= AVC_PPS_MAX_SLICE_GROUPS) {
        return ERROR_INVALID_FORMAT;
    }
    if (pps.numSliceGroupsMinus1 > 0) {
        pps.sliceGroupMapType = readGolomb(bits);
        if (pps.sliceGroupMapType == 0) {
            for (unsigned int i = 0; i < pps.numSliceGroupsMinus1; i++) {
                pps.runLengthMinus1[i] = readGolomb(bits);
            }
        } else if (pps.sliceGroupMapType == 2) {
            for (unsigned int i = 0; i < pps.numSliceGroupsMinus1; i++) {
                pps.topLeft[i] = readGolomb(bits);
                pps.bottomRight[i] = readGolomb(bits);
            }
        } else if (pps.sliceGroupMapType == 3 || pps.sliceGroupMapType == 4 || pps.sliceGroupMapType == 5) {
            pps.sliceGroupChangeDirectionFlag = bits.ReadBit();
            pps.sliceGroupChangeRateMinus1 = readGolomb(bits);
        } else if (pps.sliceGroupMapType == 6) {
            pps.picSizeInMapUnitsMinus1 = readGolomb(bits);
            if (pps.picSizeInMapUnitsMinus1 >= AVC_PPS_MAX_PIC_SIZE_IN_MAP_UNITS) {
                return ERROR_INVALID_FORMAT;
            }
            unsigned int numBitsPerSliceGroupId;
            if (pps.numSliceGroupsMinus1 + 1 > 4) {
                numBitsPerSliceGroupId = 3;
            } else if (pps.numSliceGroupsMinus1 + 1 > 2) {
                numBitsPerSliceGroupId = 2;
            } else {
                numBitsPerSliceGroupId = 1;
            }
            for (unsigned int i = 0; i < pps.picSizeInMapUnitsMinus1; i++) {
                bits.ReadBits(numBitsPerSliceGroupId);
            }
        }
    }
    pps.numRefIdx10ActiveMinus1 = readGolomb(bits);
    pps.numRefIdx11ActiveMinus1 = readGolomb(bits);
    pps.weightedPredFlag = bits.ReadBit();
    pps.weightedBipredIdc = bits.ReadBits(2);
    pps.picInitQpMinus26 = signedGolomb(readGolomb(bits));
    pps.picInitQsMinus26 = signedGolomb(readGolomb(bits));
    pps.chromaQpIndexOffset = signedGolomb(readGolomb(bits));
    pps.deblockingFilterControlPresentFlag = bits.ReadBit();
    pps.constrainedIntraPredFlag = bits.ReadBit();
    pps.redundantPicCntPresentFlag = bits.ReadBit();
    return SUCCESS;
}

Result AvcFrameParser::parseSliceHeader(const UI08 *data,
                                        unsigned int dataSize,
                                        unsigned int nalUnitType,
                                        unsigned int nalRefIdc,
                                        AvcSliceHeader &sliceHeader) {
    DataBuffer unescaped(data, dataSize);
    NalParser::unescape(unescaped);
    BitReader bits(unescaped.getData(), unescaped.getDataSize());

    sliceHeader.size = 0;
    sliceHeader.firstMbInSlice = readGolomb(bits);
    sliceHeader.sliceType = readGolomb(bits);
    sliceHeader.picParameterSetId = readGolomb(bits);
    if (sliceHeader.picParameterSetId > AVC_PPS_MAX_ID) {
        return ERROR_INVALID_FORMAT;
    }
    const AvcPictureParameterSet* pps = this->pps[sliceHeader.picParameterSetId];
    if (!pps) {
        return ERROR_INVALID_FORMAT;
    }
    const AvcSequenceParameterSet* sps = this->sps[pps->seqParameterSetId];
    if (!sps) {
        return ERROR_INVALID_FORMAT;
    }
    if (sps->seqScalingMatrixPresentFlag) {
        sliceHeader.colourPlaneId = bits.ReadBits(2);
    }
    sliceHeader.frameNum = bits.ReadBits(sps->log2MaxFrameNumMinus4 + 4);
    if (!sps->frameMbsOnlyFlag) {
        sliceHeader.fieldPicFlag = bits.ReadBit();
        if (sliceHeader.fieldPicFlag) {
            sliceHeader.bottomFieldFlag = bits.ReadBit();
        }
    }
    if (nalUnitType == AVC_NAL_UNIT_TYPE_CODED_SLICE_OF_IDR_PICTURE) {
        sliceHeader.idrPicId = readGolomb(bits);
    }
    if (sps->picOrderCntType == 0) {
        sliceHeader.picOrderCntLsb = bits.ReadBits(sps->log2MaxPicOrderCntLsbMinus4 + 4);
        if (pps->picOrderPresentFlag && !sliceHeader.fieldPicFlag) {
            sliceHeader.deltaPicOrderCnt[0] = signedGolomb(readGolomb(bits));
        }
    }
    if (sps->picOrderCntType == 1 && !sps->deltaPicOrderAlwaysZeroFlags) {
        sliceHeader.deltaPicOrderCnt[0] = signedGolomb(readGolomb(bits));
        if (pps->picOrderPresentFlag && !sliceHeader.fieldPicFlag) {
            sliceHeader.deltaPicOrderCnt[1] = signedGolomb(readGolomb(bits));
        }
    }
    if (pps->redundantPicCntPresentFlag) {
        sliceHeader.redundantPicCnt = readGolomb(bits);
    }
    unsigned int sliceType = sliceHeader.sliceType % 5;
    if (sliceType == AVC_SLICE_TYPE_B) {
        sliceHeader.directSpatialMvPredFlag = bits.ReadBit();
    }
    if (sliceType == AVC_SLICE_TYPE_P || sliceType == AVC_SLICE_TYPE_SP || sliceType == AVC_SLICE_TYPE_B) {
        sliceHeader.numRefIdxActiveOverrideFlag = bits.ReadBit();
        if (sliceHeader.numRefIdxActiveOverrideFlag) {
            sliceHeader.numRefIdxL0ActiveMinus1 = readGolomb(bits);
            if ((sliceHeader.sliceType % 5) == AVC_SLICE_TYPE_B) {
                sliceHeader.numRefIdxL1ActiveMinus1 = readGolomb(bits);
            }
        } else {
            sliceHeader.numRefIdxL0ActiveMinus1 = pps->numRefIdx10ActiveMinus1;
            sliceHeader.numRefIdxL1ActiveMinus1 = pps->numRefIdx11ActiveMinus1;
        }
    }
    if ((sliceHeader.sliceType % 5) != 2 && (sliceHeader.sliceType % 5) != 4) {
        sliceHeader.refPicListReorderingFlagL0 = bits.ReadBit();
        if (sliceHeader.refPicListReorderingFlagL0) {
            do {
                sliceHeader.reorderingOfPicNumsIdc = readGolomb(bits);
                if (sliceHeader.reorderingOfPicNumsIdc == 0 || sliceHeader.reorderingOfPicNumsIdc == 1) {
                    sliceHeader.absDiffPicNumMinus1 = readGolomb(bits);
                } else if (sliceHeader.reorderingOfPicNumsIdc == 2) {
                    sliceHeader.longTermPicNum = readGolomb(bits);
                }
            } while (sliceHeader.reorderingOfPicNumsIdc != 3);
        }
    }
    if ((sliceHeader.sliceType % 5) == 1) {
        sliceHeader.refPicListReorderingFlagL1 = bits.ReadBit();
        if (sliceHeader.refPicListReorderingFlagL1) {
            do {
                sliceHeader.reorderingOfPicNumsIdc = readGolomb(bits) ;
                if (sliceHeader.reorderingOfPicNumsIdc == 0 || sliceHeader.reorderingOfPicNumsIdc == 1) {
                    sliceHeader.absDiffPicNumMinus1 = readGolomb(bits);
                } else if (sliceHeader.reorderingOfPicNumsIdc == 2) {
                    sliceHeader.longTermPicNum = readGolomb(bits);
                }
            } while (sliceHeader.reorderingOfPicNumsIdc != 3);
        }
    }

    if ((pps->weightedPredFlag &&
         (sliceType == AVC_SLICE_TYPE_P || sliceType == AVC_SLICE_TYPE_SP)) ||
        (pps->weightedBipredIdc == 1 && sliceType == AVC_SLICE_TYPE_B)) {
        // pred_weight_table
        sliceHeader.lumaLog2WeightDenom = readGolomb(bits);

        if (sps->chromaFormatIdc != 0) {
            sliceHeader.chromaLog2WeightDenom = readGolomb(bits);
        }

        for (unsigned int i = 0; i <= sliceHeader.numRefIdxL0ActiveMinus1; i++) {
            unsigned int lumaWeightL0Flag = bits.ReadBit();
            if (lumaWeightL0Flag) {
                readGolomb(bits);
                readGolomb(bits);
            }
            if (sps->chromaFormatIdc != 0) {
                unsigned int chromaWeightL0Flag = bits.ReadBit();
                if (chromaWeightL0Flag) {
                    for (unsigned int j = 0; j < 2; j++) {
                        readGolomb(bits);
                        readGolomb(bits);
                    }
                }
            }
        }
        if ((sliceHeader.sliceType % 5) == 1) {
            for (unsigned int i = 0; i <= sliceHeader.numRefIdxL1ActiveMinus1; i++) {
                unsigned int lumaWeightL1Flag = bits.ReadBit();
                if (lumaWeightL1Flag) {
                    readGolomb(bits);
                    readGolomb(bits);
                }
                if (sps->chromaFormatIdc != 0) {
                    unsigned int chromaWeightL1Flag = bits.ReadBit();
                    if (lumaWeightL1Flag) {
                        for (unsigned int j = 0; j < 2; j++) {
                            readGolomb(bits);
                            readGolomb(bits);
                        }
                    }
                }
            }
        }
    }

    if (nalRefIdc != 0) {
        // dec_ref_pic_marking
        if (nalUnitType == AVC_NAL_UNIT_TYPE_CODED_SLICE_OF_IDR_PICTURE) {
            sliceHeader.noOutputOfPriorPicsFlag = bits.ReadBit();
            sliceHeader.longTermReferenceFlag = bits.ReadBit();
        } else {
            unsigned int adaptiveRefPicMarkingModeFlag = bits.ReadBit();
            if (adaptiveRefPicMarkingModeFlag) {
                unsigned int memoryManagementControlOperation = 0;
                do {
                    memoryManagementControlOperation = readGolomb(bits);
                    if (memoryManagementControlOperation == 1 || memoryManagementControlOperation == 3) {
                        sliceHeader.differenceOfPicNumsMinus1 = readGolomb(bits);
                    }
                    if (memoryManagementControlOperation == 2) {
                        sliceHeader.longTermPicNum = readGolomb(bits);
                    }
                    if (memoryManagementControlOperation == 3 || memoryManagementControlOperation == 6) {
                        sliceHeader.longTermFrameIdx = readGolomb(bits);
                    }
                    if (memoryManagementControlOperation == 4) {
                        sliceHeader.maxLongTermFrameIdxPlus1 = readGolomb(bits);
                    }
                } while (memoryManagementControlOperation != 0);
            }
        }
    }
    if (pps->entropyCodingModeFlag && sliceType != AVC_SLICE_TYPE_I && sliceType != AVC_SLICE_TYPE_SI) {
        sliceHeader.cabacInitIdc = readGolomb(bits);
    }
    sliceHeader.sliceQpDelta = readGolomb(bits);
    if (sliceType == AVC_SLICE_TYPE_SP || sliceType == AVC_SLICE_TYPE_SI) {
        if (sliceType == AVC_SLICE_TYPE_SP) {
            sliceHeader.spForSwitchFlag = bits.ReadBit();
        }
        sliceHeader.sliceQsDelta = signedGolomb(readGolomb(bits));
    }
    if (pps->deblockingFilterControlPresentFlag) {
        sliceHeader.disableDeblockingFilterIdc = readGolomb(bits);
        if (sliceHeader.disableDeblockingFilterIdc != 1) {
            sliceHeader.sliceAlphaC0OffsetDiv2 = signedGolomb(readGolomb(bits));
            sliceHeader.sliceBetaOffsetDiv2 = signedGolomb(readGolomb(bits));
        }
    }
    if (pps->numSliceGroupsMinus1 > 0 &&
        pps->sliceGroupMapType >= 3   &&
        pps->sliceGroupMapType <= 5) {
        sliceHeader.sliceGroupChangeCycle = readGolomb(bits);
    }

    /* compute the size */
    sliceHeader.size = bits.GetBitsRead();

    return 0;
}

bool AvcFrameParser::sameFrame(unsigned int nalUnitType1, unsigned int nalRefIdc1, AvcSliceHeader &sh1,
                               unsigned int nalUnitType2, unsigned int nalRefIdc2, AvcSliceHeader &sh2) {
    if (sh1.frameNum != sh2.frameNum) {
        return false;
    }
    if (sh1.picParameterSetId != sh2.picParameterSetId) {
        return false;
    }
    if (sh1.fieldPicFlag != sh2.fieldPicFlag) {
        return false;
    }
    if (sh1.fieldPicFlag) {
        if (sh1.bottomFieldFlag != sh2.bottomFieldFlag) {
            return false;
        }
    }
    if ((nalRefIdc1 == 0 || nalRefIdc2 == 0) && (nalRefIdc1 != nalRefIdc2)) {
        return false;
    }
    auto* sps = getSliceSps(sh1);
    if (!sps) {
        return false;
    }
    if (sps->picOrderCntType == 0) {
        if (sh1.picOrderCntLsb != sh2.picOrderCntLsb || sh1.deltaPicOrderCnt[0] != sh2.deltaPicOrderCnt[0]) {
            return false;
        }
    }
    if (sps->picOrderCntType == 1) {
        if (sh1.deltaPicOrderCnt[0] != sh2.deltaPicOrderCnt[0] || sh1.deltaPicOrderCnt[1] != sh2.deltaPicOrderCnt[1]) {
            return false;
        }
    }
    if (nalUnitType1 == AVC_NAL_UNIT_TYPE_CODED_SLICE_OF_IDR_PICTURE || nalUnitType2 == AVC_NAL_UNIT_TYPE_CODED_SLICE_OF_IDR_PICTURE) {
        if (nalUnitType1 != nalUnitType2) {
            return false;
        }
    }
    if (nalUnitType1 == AVC_NAL_UNIT_TYPE_CODED_SLICE_OF_IDR_PICTURE || nalUnitType2 == AVC_NAL_UNIT_TYPE_CODED_SLICE_OF_IDR_PICTURE) {
        if (sh1.idrPicId != sh2.idrPicId) {
            return false;
        }
    }
    return true;
}

AvcSequenceParameterSet *AvcFrameParser::getSliceSps(AvcSliceHeader &sh) {
    auto* pps = this->pps[sh.picParameterSetId];
    if (!pps) {
        return nullptr;
    }
    return this->sps[pps->seqParameterSetId];
}

void AvcFrameParser::checkIfAccessUnitIsCompleted(AvcFrameParser::AccessUnitInfo &accessUnitInfo) {
    if (!sliceHeader) {
        return;
    }
    if (!accessUnitVclNalUnitCount) {
        return;
    }
//    debug("New Access Unit \n");
    this->accessUnitVclNalUnitCount = 0;
    auto* sps = getSliceSps(*sliceHeader);
    if (!sps) {
        return;
    }
    int maxFrameNum = 1 << (sps->log2MaxFrameNumMinus4 + 4);
    enum {
        AVC_PIC_TYPE_FRAME,
        AVC_PIC_TYPE_TOP_FIELD,
        AVC_PIC_TYPE_BOTTOM_FIELD,
    } picType;
    if (sps->frameMbsOnlyFlag || !sliceHeader->fieldPicFlag) {
        picType = AVC_PIC_TYPE_FRAME;
    } else if (sliceHeader->bottomFieldFlag) {
        picType = AVC_PIC_TYPE_BOTTOM_FIELD;
    } else {
        picType = AVC_PIC_TYPE_TOP_FIELD;
    }
    int topFieldPicOrderCnt = 0;
    int bottomFieldPicOrderCnt = 0;
    unsigned int frameNumOffset = 0;
    unsigned int frameNum = sliceHeader->frameNum;
    if (this->nalUnitType == AVC_NAL_UNIT_TYPE_CODED_SLICE_OF_IDR_PICTURE) {
        this->prevPicOrderCntLsb = 0;
        this->prevPicOrderCntMsb = 0;
    } else {
        if (frameNum < this->prevFrameNum) {
            frameNumOffset = this->prevFrameNumOffset + maxFrameNum;
        } else {
            frameNumOffset = this->prevFrameNumOffset;
        }
    }
    int picOrderCntMsb = 0;
    if (sps->picOrderCntType == 0) {
        unsigned int maxPicOrderCntLsb = 1 << (sps->log2MaxPicOrderCntLsbMinus4 + 4);
        if (this->sliceHeader->picOrderCntLsb < this->prevPicOrderCntLsb &&
                this->prevPicOrderCntLsb - this->sliceHeader->picOrderCntLsb >= maxPicOrderCntLsb / 2) {
            picOrderCntMsb = this->prevPicOrderCntMsb + maxPicOrderCntLsb;
        } else if (this->sliceHeader->picOrderCntLsb > this->prevPicOrderCntLsb &&
                this->sliceHeader->picOrderCntLsb - this->prevPicOrderCntLsb > maxPicOrderCntLsb / 2) {
            picOrderCntMsb = this->prevPicOrderCntMsb - maxPicOrderCntLsb;
        } else {
            picOrderCntMsb = this->prevPicOrderCntMsb;
        }

        if (picType != AVC_PIC_TYPE_BOTTOM_FIELD) {
            topFieldPicOrderCnt = picOrderCntMsb + this->sliceHeader->picOrderCntLsb;
        }
        if (picType != AVC_PIC_TYPE_TOP_FIELD) {
            if (!this->sliceHeader->fieldPicFlag) {
                bottomFieldPicOrderCnt = topFieldPicOrderCnt + this->sliceHeader->deltaPicOrderCnt[0];
            } else {
                bottomFieldPicOrderCnt = picOrderCntMsb + this->sliceHeader->picOrderCntLsb;
            }
        }
    } else if (sps->picOrderCntType == 1) {
        unsigned int absFrameNum = 0;
        if (sps->numRefFramesInPicOrderCntCycle) {
            absFrameNum = frameNumOffset + frameNum;
        }
        if (this->nalRefIdc == 0 && absFrameNum == 0) {
            --absFrameNum;
        }
        int expectedPicOrderCnt = 0;
        if (absFrameNum > 0) {
            unsigned int picOrderCntCycleCnt = (absFrameNum - 1) / sps->numRefFramesInPicOrderCntCycle;
            unsigned int frameNumInPicOrderCntCycle = (absFrameNum - 1) % sps->numRefFramesInPicOrderCntCycle;
            int expectedDeltaPerPicOrderCntCycle = 0;
            for (unsigned int i = 0; i < sps->numRefFramesInPicOrderCntCycle; i++) {
                expectedDeltaPerPicOrderCntCycle += sps->offsetForRefFrame[i];
            }
            expectedPicOrderCnt = picOrderCntCycleCnt * expectedDeltaPerPicOrderCntCycle;
            for (unsigned int i = 0; i < frameNumInPicOrderCntCycle; i++) {
                expectedPicOrderCnt += sps->offsetForRefFrame[i];
            }
        }
        if (this->nalRefIdc == 0) {
            expectedPicOrderCnt += sps->offsetForNonRefPic;
        }
        if (!this->sliceHeader->fieldPicFlag) {
            topFieldPicOrderCnt = expectedPicOrderCnt + this->sliceHeader->deltaPicOrderCnt[0];
            bottomFieldPicOrderCnt = topFieldPicOrderCnt + sps->offsetForTopToBottomField + this->sliceHeader->deltaPicOrderCnt[1];
        } else if (!this->sliceHeader->bottomFieldFlag) {
            topFieldPicOrderCnt = expectedPicOrderCnt + this->sliceHeader->deltaPicOrderCnt[0];
        } else {
            bottomFieldPicOrderCnt = expectedPicOrderCnt + sps->offsetForTopToBottomField + this->sliceHeader->deltaPicOrderCnt[0];
        }
    } else if (sps->picOrderCntType == 2) {
        int picOrderCount;
        if (this->nalUnitType == AVC_NAL_UNIT_TYPE_CODED_SLICE_OF_IDR_PICTURE) {
            picOrderCount = 0;
        } else if (this->nalRefIdc == 0) {
            picOrderCount = 2 * (frameNumOffset + frameNum) - 1;
        } else {
            picOrderCount = 2 * (frameNumOffset + frameNum);
        }
        if (!this->sliceHeader->fieldPicFlag) {
            topFieldPicOrderCnt = picOrderCount;
        } else if (this->sliceHeader->bottomFieldFlag) {
            bottomFieldPicOrderCnt = picOrderCount;
        } else {
            topFieldPicOrderCnt = picOrderCount;
        }
    }
    unsigned int picOrderCount;
    if (picType == AVC_PIC_TYPE_FRAME) {
        picOrderCount = topFieldPicOrderCnt < bottomFieldPicOrderCnt ? topFieldPicOrderCnt : bottomFieldPicOrderCnt;
    } else if (picType == AVC_PIC_TYPE_TOP_FIELD) {
        picOrderCount = topFieldPicOrderCnt;
    } else {
        picOrderCount = bottomFieldPicOrderCnt;
    }
    accessUnitInfo.nalUnits = this->accessUnitData;
    accessUnitInfo.isIdr = this->nalUnitType == AVC_NAL_UNIT_TYPE_CODED_SLICE_OF_IDR_PICTURE;
    accessUnitInfo.decodeOrder = this->totalAccessUnitCount;
    accessUnitInfo.displayOrder = picOrderCount;
    this->accessUnitData.Clear();
    ++this->totalAccessUnitCount;

    this->prevFrameNum = frameNum;
    this->prevFrameNumOffset = frameNumOffset;
    if (this->nalRefIdc) {
        this->prevPicOrderCntMsb = picOrderCntMsb;
        this->prevPicOrderCntLsb = this->sliceHeader->picOrderCntLsb;
    }
}

void AvcFrameParser::appendNalUnitData(const unsigned char *data, unsigned int dataSize) {
    accessUnitData.Append(new DataBuffer(data, dataSize));
}

void AvcFrameParser::AccessUnitInfo::reset() {
    for (unsigned int i = 0; i < nalUnits.ItemCount(); i++) {
        delete nalUnits[i];
    }
    nalUnits.Clear();
    isIdr = false;
    decodeOrder = 0;
    displayOrder = 0;
}
