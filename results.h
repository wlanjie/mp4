//
// Created by wlanjie on 2018/2/7.
//

#ifndef MP4_RESULTS_H
#define MP4_RESULTS_H

namespace mp4 {

const int SUCCESS                               =  0;
const int FAILURE                               = -1;
const int ERROR_OUT_OF_MEMORY                   = -2;
const int ERROR_INVALID_PARAMETERS              = -3;
const int ERROR_NO_SUCH_FILE                    = -4;
const int ERROR_PERMISSION_DENIED               = -5;
const int ERROR_CANNOT_OPEN_FILE                = -6;
const int ERROR_EOS                             = -7;
const int ERROR_WRITE_FAILED                    = -8;
const int ERROR_READ_FAILED                     = -9;
const int ERROR_INVALID_FORMAT                  = -10;
const int ERROR_NO_SUCH_ITEM                    = -11;
const int ERROR_OUT_OF_RANGE                    = -12;
const int ERROR_INTERNAL                        = -13;
const int ERROR_INVALID_STATE                   = -14;
const int ERROR_LIST_EMPTY                      = -15;
const int ERROR_LIST_OPERATION_ABORTED          = -16;
const int ERROR_INVALID_RTP_CONSTRUCTOR_TYPE    = -17;
const int ERROR_NOT_SUPPORTED                   = -18;
const int ERROR_INVALID_TRACK_TYPE              = -19;
const int ERROR_INVALID_RTP_PACKET_EXTRA_DATA   = -20;
const int ERROR_BUFFER_TOO_SMALL                = -21;
const int ERROR_NOT_ENOUGH_DATA                 = -22;
const int ERROR_NOT_ENOUGH_SPACE                = -23;

const char* resultText(int result);
// TODO define
#define FAILED(result) ((result) != SUCCESS)
#define SUCCEEDED(result) ((result) == SUCCESS)

#define CHECK(_x) do {                      \
    Result _result = (_x);                  \
    if (FAILED(_result)) return _result;    \
} while(0)

}
#endif //MP4_RESULTS_H
