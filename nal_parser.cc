//
// Created by wlanjie on 2018/3/13.
//

#include "nal_parser.h"
#include "utils.h"

void NalParser::unescape(DataBuffer &data) {
    unsigned int zeroCount = 0;
    unsigned int bytesRemoved = 0;
    UI08* out = data.useData();
    const UI08* in = data.getData();
    Size inSize = data.getDataSize();
    for (unsigned int i = 0; i < inSize; i++) {
        if (zeroCount >= 2 && in[i] == 3 && i + 1 < inSize && in[i + 1] <= 3) {
            ++bytesRemoved;
            zeroCount = 0;
        } else {
            out[i - bytesRemoved] = in[i];
            if (in[i] == 0) {
                ++zeroCount;
            }
        }
    }
    data.setDataSize(inSize - bytesRemoved);
}

NalParser::NalParser() : State(STATE_RESET), zeroTrail(0) {

}

Result NalParser::feed(const void *data, Size dataSize, Size &bytesConsumed, const DataBuffer *&nalu, bool eos) {
    nalu = nullptr;
    bytesConsumed = 0;
    unsigned int dataOffset;
    unsigned int payloadStart = 0;
    unsigned int payloadEnd = 0;
    bool foundNalu = false;
    for (dataOffset = 0; dataOffset < dataSize && !foundNalu; dataOffset++) {
        unsigned char byte = ((const unsigned char*) data)[dataOffset];
        switch (State) {
            case STATE_RESET:
                if (byte == 0) {
                    State = STATE_START_CODE_1;
                }
                break;

            case STATE_START_CODE_1:
                if (byte == 0) {
                    State = STATE_START_CODE_2;
                } else {
                    State = STATE_RESET;
                }
                break;

            case STATE_START_CODE_2:
                if (byte == 0) {
                    break;
                }
                if (byte == 1) {
                    State = STATE_START_NALU;
                } else {
                    State = STATE_RESET;
                }
                break;

            case STATE_START_NALU:
                buffer.setDataSize(0);
                zeroTrail = 0;
                payloadStart = payloadEnd = dataOffset;
                State = STATE_IN_NALU;
//                break;

            case STATE_IN_NALU:
                if (byte == 0) {
                    ++zeroTrail;
                    ++payloadEnd;
                    break;
                }
                if (zeroTrail >= 2) {
                    if (byte == 1) {
                        foundNalu = true;
                        State = STATE_START_NALU;
                        break;
                    } else {
                        ++payloadEnd;
                    }
                } else {
                    ++payloadEnd;
                }
                zeroTrail = 0;
                break;
        }
    }
    if (eos && State == STATE_IN_NALU && dataOffset == dataSize) {
        foundNalu = true;
        zeroTrail = 0;
        State = STATE_RESET;
    }
    if (payloadEnd > payloadStart) {
        Size currentPayloadSize = buffer.getDataSize();
        buffer.setDataSize(buffer.getDataSize() + (payloadEnd - payloadStart));
        CopyMemory(((unsigned char *) buffer.useData()) + currentPayloadSize,
                   ((const unsigned char*) data) + payloadStart,
                   payloadEnd - payloadStart);
    }
    bytesConsumed = dataOffset;
    if (foundNalu) {
        if (zeroTrail >= 3 && buffer.getDataSize() >= 3) {
            buffer.setDataSize(buffer.getDataSize() - 3);
        } else if (zeroTrail >= 2 && buffer.getDataSize() >= 2) {
            buffer.setDataSize(buffer.getDataSize() - 2);
        }
        zeroTrail = 0;
        nalu = &buffer;
    }
    return SUCCESS;
}

Result NalParser::reset() {
    State = STATE_RESET;
    zeroTrail = 0;
    buffer.setDataSize(0);
    return SUCCESS;
}
