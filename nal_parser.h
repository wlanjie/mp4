//
// Created by wlanjie on 2018/3/13.
//

#ifndef MP4_NALPARSER_H
#define MP4_NALPARSER_H


#include "databuffer.h"

class NalParser {
public:
    static void unescape(DataBuffer& data);

    NalParser();

    /**
    * Feed some data to the parser and look for the next NAL Unit.
    *
    * @param data Pointer to the memory buffer with the data to feed.
    * @param data_size Size in bytes of the buffer pointed to by the
    * data pointer.
    * @param bytes_consumed Number of bytes from the data buffer that were
    * consumed and stored by the parser.
    * @param nalu Reference to a pointer to a buffer object that contains
    * a NAL unit found in the previously fed data, or a NULL pointer if no
    * NAL unit can be found so far.
    * @param eos Boolean flag that indicates if this buffer is the last
    * buffer in the stream/file (End Of Stream).
    *
    * @result: AP4_SUCCESS is the call succeeds, or an error code if it
    * fails.
    *
    * The caller must not feed the same data twice. When this method
    * returns, the caller should inspect the value of bytes_consumed and
    * advance the input stream source accordingly, such that the next
    * buffer passed to this method will be exactly bytes_consumed bytes
    * after what was passed in this call.
    */
    Result feed(const void* data,
                    Size dataSize,
                    Size& bytesConsumed,
                    const DataBuffer*& nalu,
                    bool eos=false);

    /**
     * Reset the state of the parser (for example, to parse a new stream).
     */
    Result reset();

protected:
    enum {
        STATE_RESET,
        STATE_START_CODE_1,
        STATE_START_CODE_2,
        STATE_START_NALU,
        STATE_IN_NALU
    } State;
    Cardinal zeroTrail;
    DataBuffer buffer;

};


#endif //MP4_NALPARSER_H
